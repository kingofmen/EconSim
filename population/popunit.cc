#include "population/popunit.h"

#include <cmath>
#include <limits>
#include <list>
#include <iostream>
#include "absl/algorithm/container.h"
#include "geography/proto/geography.pb.h"
#include "industry/industry.h"
#include "util/arithmetic/microunits.h"
#include "util/keywords/keywords.h"

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

market::proto::Container TotalNeeded(const proto::ConsumptionPackage& package,
                                     int size) {
  auto needed = package.consumed();
  needed += package.capital();
  needed *= size;
  return needed;
}

// Attempts to buy each good in the provided basket from market, paying with
// resources. Returns true if all the buys are successful.
bool BuyBasket(const market::proto::Container& basket,
               market::proto::Container* resources, market::Market* market) {
  bool success = true;
  for (const auto& need : basket.quantities()) {
    if (need.second <= 0) {
      continue;
    }
    auto bought = market->TryToBuy(need.first, need.second, resources);
    if (bought < need.second) {
      success = false;
    }
  }
  return success;
}

void BuyPackage(const proto::ConsumptionPackage& package, int size,
                market::proto::Container* resources, market::Market* market) {
  auto needed = TotalNeeded(package, size);
  needed -= *resources;
  BuyBasket(needed, resources, market);
}

} // namespace

std::unordered_map<uint64, PopUnit*> PopUnit::id_to_pop_map_;

PopUnit::PopUnit() : evaluator_(&default_evaluator_), packages_ordered_(0) {
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
    market::Market* market) {
  const proto::AutoProduction* best_prod = nullptr;
  market::Measure best_price = 0;
  for (const auto* p : production) {
    if (!(proto_.tags() > p->required_tags())) {
      continue;
    }
    auto curr_price = market->GetPriceU(p->output());
    if (curr_price < best_price) {
      continue;
    }
    best_price = curr_price;
    best_prod = p;
  }
  if (best_prod == nullptr) {
    return;
  }
  *mutable_wealth() += best_prod->output() * GetSize();
  SellSurplus(market);
}

void PopUnit::BirthAndDeath() {}

const proto::ConsumptionPackage*
PopUnit::CheapestPackage(const proto::ConsumptionLevel& level,
                         const market::Market& market,
                         const proto::ConsumptionPackage*& cheapest) const {
  const proto::ConsumptionPackage* best_package = nullptr;
  market::Measure best_price_u = std::numeric_limits<market::Measure>::max();
  const int size = GetSize();
  auto max_money = market.MaxMoney(proto_.wealth());
  for (const auto& package : level.packages()) {
    if (!(proto_.tags() > package.required_tags())) {
      continue;
    }

    auto needed = TotalNeeded(package, size);
    if (proto_.wealth() > needed) {
      auto curr_price_u = market.GetPriceU(package.consumed());
      if (curr_price_u < best_price_u) {
        best_package = &package;
        cheapest = &package;
        best_price_u = curr_price_u;
      }
      continue;
    }
    bool can_buy = true;
    market::Measure package_money = 0;
    for (const auto& quantity : needed.quantities()) {
      auto need_to_buy =
          quantity.second - market::GetAmount(proto_.wealth(), quantity.first);
      if (market.AvailableImmediately(quantity.first) >= need_to_buy) {
        package_money +=
            micro::MultiplyU(need_to_buy, market.GetPriceU(quantity.first));
        if (package_money <= max_money) {
          continue;
        }
      }
      can_buy = false;
      break;
    }

    needed -= proto_.wealth();
    auto curr_price_u = market.GetPriceU(needed);
    if (curr_price_u < best_price_u) {
      if (can_buy) {
        best_package = &package;
      }
      cheapest = &package;
      best_price_u = curr_price_u;
    }
  }

  return best_package;
}

bool PopUnit::Consume(const proto::ConsumptionLevel& level,
                      market::Market* market) {
  const proto::ConsumptionPackage* cheapest = nullptr;
  const proto::ConsumptionPackage* best_package =
      CheapestPackage(level, *market, cheapest);
  auto* resources = mutable_wealth();
  int size = GetSize();

  if (best_package == nullptr) {
    if (cheapest != nullptr && packages_ordered_ == 0) {
      ++packages_ordered_;
      BuyPackage(*cheapest, size, resources, market);
    }
    return false;
  }

  BuyPackage(*best_package, size, resources, market);
  *resources -= best_package->consumed() * size;

  *proto_.mutable_tags() += best_package->tags();
  *proto_.mutable_tags() += level.tags();
  return true;
}

void PopUnit::EndTurn(const market::proto::Container& decay_rates_u) {
  micro::MultiplyU(*mutable_wealth(), decay_rates_u);
  micro::MultiplyU(*proto_.mutable_tags(), decay_rates_u);
  market::CleanContainer(mutable_wealth());
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
  // First step info is for current step.
  const auto& step_info = production_info.step_info(0);
  auto variant_index = step_info.best_variant();
  if (variant_index >= step_info.variant_size()) {
    return false;
  }
  const auto& variant_info = step_info.variant(variant_index);
  if (progress->scaling_u() > variant_info.possible_scale_u()) {
    progress->set_scaling_u(variant_info.possible_scale_u());
  }

  auto required = production.RequiredConsumables(*progress, variant_index);
  required -= wealth();
  if (!BuyBasket(required, mutable_wealth(), market)) {
    return false;
  }

  production.PerformStep(field->fixed_capital(), 0, variant_index,
                         mutable_wealth(), field->mutable_resources(),
                         mutable_wealth(), progress);

  fields_worked_.insert(field);
  if (production.Complete(*progress)) {
    field->clear_progress();
    SellSurplus(market);
  }

  return true;
}

void PopUnit::StartTurn(
    const std::vector<const proto::ConsumptionLevel*>& levels,
    market::Market* market) {
  packages_ordered_ = 0;
  fields_worked_.clear();
  subsistence_need_.Clear();
  market::Measure found = 0;
  const proto::ConsumptionPackage* cheapest = nullptr;
  for (const auto* level : levels) {
    auto amount = market::GetAmount(level->tags(), keywords::kSubsistenceTag);
    if (amount <= 0) {
      continue;
    }
    const proto::ConsumptionPackage* best_package =
        CheapestPackage(*level, *market, cheapest);
    if (best_package == nullptr) {
      best_package = cheapest;
    }
    if (best_package == nullptr) {
      continue;
    }
    subsistence_need_ += TotalNeeded(*best_package, GetSize());
    found += amount;
    if (found > 1) {
      break;
    }
  }
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
      best_chain->MakeProgress(decision.selected().max_scale_u());
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

void PopUnit::SellSurplus(market::Market* market) {
  for (const auto& quantity : wealth().quantities()) {
    auto amount = quantity.second;
    amount -= market::GetAmount(subsistence_need_, quantity.first);
    if (amount > 0) {
      market->TryToSell(market::MakeQuantity(quantity.first, amount),
                        mutable_wealth());
    }
  }
}

uint64 PopUnit::NewPopId() { return ++unused_pop_id; }

} // namespace population
