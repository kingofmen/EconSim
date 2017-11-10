#include "population/popunit.h"

#include <cmath>
#include <limits>
#include <list>

#include "absl/algorithm/container.h"
#include "geography/proto/geography.pb.h"
#include "industry/industry.h"

namespace population {

using industry::proto::Production;

industry::decisions::ProductionEvaluator& PopUnit::default_evaluator_ =
    industry::decisions::LocalProfitMaximiser();

namespace {
uint64 unused_pop_id = 1;
constexpr int kNumAgeGroups = 7;
constexpr char kExpectedProfit[] = "expected_profit";
constexpr char kStandardDeviation[] = "standard_deviation";
constexpr char kCorrelation[] = "correlation";
constexpr char kSubsistence[] = "subsistence";

market::proto::Container TotalNeeded(const proto::ConsumptionPackage& package,
                                     int size) {
  auto needed = package.food().consumed();
  needed += package.food().capital();
  needed += package.shelter().consumed();
  needed += package.shelter().capital();
  needed += package.culture().consumed();
  needed += package.culture().capital();
  needed *= size;
  return needed;
}

} // namespace

std::unordered_map<uint64, PopUnit*> PopUnit::id_to_pop_map_;

PopUnit::PopUnit() : evaluator_(&default_evaluator_) {
  proto_.set_pop_id(NewPopId());
  for (int i = 0; i < kNumAgeGroups; ++i) {
    proto_.add_males(0);
    proto_.add_women(0);
  }
  id_to_pop_map_[proto_.pop_id()] = this;
}

PopUnit::PopUnit(const proto::PopUnit& proto)
    : proto_(proto), evaluator_(&default_evaluator_) {
  if (id_to_pop_map_[proto_.pop_id()] != nullptr) {
    // TODO: Error here.
  }
  id_to_pop_map_[proto_.pop_id()] = this;
}

void PopUnit::AutoProduce(
    const std::vector<const proto::AutoProduction*>& production,
    const market::proto::Container& prices) {
  const proto::AutoProduction* best_prod = nullptr;
  double best_price = 0;
  for (const auto* p : production) {
    if (!(proto_.tags() > p->required_tags())) {
      continue;
    }
    double curr_price = p->output() * prices;
    if (curr_price < best_price) {
      continue;
    }
    best_price = curr_price;
    best_prod = p;
  }
  if (best_prod == nullptr) {
    return;
  }
  *proto_.mutable_wealth() += best_prod->output();
}

void PopUnit::BirthAndDeath() {}

const proto::ConsumptionPackage*
PopUnit::CheapestPackage(const proto::ConsumptionLevel& level,
                         const market::proto::Container& prices) const {
  const proto::ConsumptionPackage* best_package = nullptr;
  double best_price = std::numeric_limits<double>::max();
  int size = GetSize();
  for (const auto& package : level.packages()) {
    if (!(proto_.tags() > package.required_tags())) {
      continue;
    }

    if (proto_.wealth() > TotalNeeded(package, size)) {
      double curr_price = prices * package.food().consumed();
      curr_price += prices * package.shelter().consumed();
      curr_price += prices * package.culture().consumed();
      if (curr_price < best_price) {
        best_package = &package;
        best_price = curr_price;
      }
    }
  }
  return best_package;
}

bool PopUnit::Consume(const proto::ConsumptionLevel& level,
                      const market::proto::Container& prices) {
  const proto::ConsumptionPackage* best_package =
      CheapestPackage(level, prices);
  if (best_package == nullptr) {
    return false;
  }

  auto& resources = *proto_.mutable_wealth();
  int size = GetSize();
  resources -= best_package->food().consumed() * size;
  resources -= best_package->shelter().consumed() * size;
  resources -= best_package->culture().consumed() * size;

  *proto_.mutable_tags() += best_package->tags();
  *proto_.mutable_tags() += level.tags();
  return true;
}

void PopUnit::EndTurn(const market::proto::Container& decay_rates) {
  fields_worked_.clear();
  *proto_.mutable_wealth() *= decay_rates;
  market::CleanContainer(proto_.mutable_wealth());
}

int PopUnit::GetSize() const {
  int size = 0;
  for (const auto men : proto_.males()) {
    size += men;
  }
  for (const auto females : proto_.women()) {
    size += females;
  }
  return size;
}

bool PopUnit::TryProductionStep(
    const industry::Production& production,
    const industry::decisions::proto::ProductionInfo& production_info,
    geography::proto::Field* field, industry::proto::Progress* progress,
    market::Market* market) {
  if (absl::c_find(fields_worked_, field) != fields_worked_.end()) {
    return false;
  }
  const auto& step_info = production_info.step_info(progress->step());
  auto variant_index = step_info.best_variant();
  if (variant_index >= step_info.variant_size()) {
    return false;
  }
  const auto& variant_info = step_info.variant(variant_index);
  if (progress->scaling() > variant_info.possible_scale()) {
    progress->set_scaling(variant_info.possible_scale());
  }
  auto required = production.RequiredConsumables(*progress, variant_index);
  for (const auto& good : required.quantities()) {
    double amount_to_buy =
        good.second - market::GetAmount(proto_.wealth(), good.first);
    if (amount_to_buy > 0) {
      double bought =
          market->TryToBuy(good.first, amount_to_buy, proto_.mutable_wealth());
      if (bought < amount_to_buy) {
        // This should never happen.
        return false;
      }
    }
  }

  production.PerformStep(field->fixed_capital(), 0.0, variant_index,
                         proto_.mutable_wealth(), field->mutable_resources(),
                         proto_.mutable_wealth(), progress);

  fields_worked_.insert(field);
  return true;
}

bool PopUnit::StartNewProduction(
    const industry::decisions::ProductionContext& context,
    industry::decisions::DecisionMap* production_info_map,
    geography::proto::Field* field) {
  auto decision = evaluator_->Evaluate(context, proto_.wealth(), field);
  production_info_map->emplace(field, decision);
  if (!decision.has_selected()) {
    return false;
  }

  auto* best_chain = context.production_map.at(decision.selected().name());
  *field->mutable_progress() =
      best_chain->MakeProgress(decision.selected().max_scale());
  return true;
}

bool PopUnit::Produce(const industry::decisions::ProductionContext& context,
                      industry::decisions::DecisionMap* production_info_map) {
  bool any_progress = false;

  for (auto* field : context.fields) {
    if (absl::c_find(fields_worked_, field) != fields_worked_.end()) {
      continue;
    }
    if (!field->has_progress()) {
      if (!StartNewProduction(context, production_info_map, field)) {
        continue;
      }
    }

    auto* progress = field->mutable_progress();
    const auto chain = context.production_map.find(progress->name());
    if (chain == context.production_map.end()) {
      // TODO: Error here!
      continue;
    }
    if (production_info_map->find(field) == production_info_map->end()) {
      production_info_map->emplace(
          field, evaluator_->Evaluate(context, proto_.wealth(), field));
    }
    if (TryProductionStep(*chain->second,
                          production_info_map->at(field).selected(), field,
                          progress, context.market)) {
      any_progress = true;
    }
  }
  return any_progress;
}

uint64 PopUnit::NewPopId() { return ++unused_pop_id; }

} // namespace population
