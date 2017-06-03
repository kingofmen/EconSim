#include "population/popunit.h"

#include <cmath>
#include <limits>
#include <list>

#include "geography/geography.h"
#include "industry/industry.h"

namespace population {

using industry::proto::Production;

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

PopUnit::PopUnit() {
  set_pop_id(NewPopId());
  for (int i = 0; i < kNumAgeGroups; ++i) {
    add_males(0);
    add_women(0);
  }
  id_to_pop_map_[pop_id()] = this;
}

PopUnit::PopUnit(const proto::PopUnit& proto) : proto::PopUnit(proto) {
  if (id_to_pop_map_[pop_id()] != nullptr) {
    // TODO: Error here.
  }
  id_to_pop_map_[pop_id()] = this;
}

void PopUnit::AutoProduce(
    const std::vector<const proto::AutoProduction*>& production,
    const market::proto::Container& prices) {
  const proto::AutoProduction* best_prod = nullptr;
  double best_price = 0;
  for (const auto* p : production) {
    if (!(tags() > p->required_tags())) {
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
  *mutable_wealth() += best_prod->output();
}

void PopUnit::BirthAndDeath() {}

const proto::ConsumptionPackage*
PopUnit::CheapestPackage(const proto::ConsumptionLevel& level,
                         const market::proto::Container& prices) const {
  const proto::ConsumptionPackage* best_package = nullptr;
  double best_price = std::numeric_limits<double>::max();
  int size = GetSize();
  for (const auto& package : level.packages()) {
    if (!(tags() > package.required_tags())) {
      continue;
    }

    if (wealth() > TotalNeeded(package, size)) {
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

  auto& resources = *mutable_wealth();
  int size = GetSize();
  resources -= best_package->food().consumed() * size;
  resources -= best_package->shelter().consumed() * size;
  resources -= best_package->culture().consumed() * size;

  *mutable_tags() += best_package->tags();
  *mutable_tags() += level.tags();
  return true;
}

void PopUnit::DecayWealth(const market::proto::Container& decay_rates) {
  *mutable_wealth() *= decay_rates;
  market::CleanContainer(mutable_wealth());
}

int PopUnit::GetSize() const {
  int size = 0;
  for (const auto men : males()) {
    size += men;
  }
  for (const auto females : women()) {
    size += females;
  }
  return size;
}

void PopUnit::GetStepInfo(const industry::Production& production,
                          const market::Market& market,
                          const geography::proto::Field& field,
                          const industry::proto::Progress& progress,
                          PopUnit::ProductionStepInfo* step_info) const {
  const auto& step = production.steps(progress.step());
  for (int index = 0; index < step.variants_size(); ++index) {
    step_info->variants.emplace_back();
    auto& variant_info = step_info->variants.back();
    const auto& input = step.variants(index);
    if (!(field.fixed_capital() > input.fixed_capital())) {
      continue;
    }
    variant_info.possible_scale = progress.scaling();
    for (const auto& good : input.raw_materials().quantities()) {
      double ratio = market::GetAmount(field.resources(), good.first);
      ratio /= good.second;
      if (ratio < variant_info.possible_scale) {
        variant_info.possible_scale = ratio;
      }
    }

    auto required = production.RequiredConsumables(progress.step(), index);
    for (const auto& good : required.quantities()) {
      variant_info.unit_cost += market.GetPrice(good.first) * good.second;
      double ratio = market.AvailableImmediately(good.first) +
                     market::GetAmount(wealth(), good.first);
      ratio /= good.second;
      if (ratio < variant_info.possible_scale) {
        variant_info.possible_scale = ratio;
      }
    }
  }
}

bool PopUnit::TryProductionStep(const industry::Production& production,
                                geography::proto::Field* field,
                                industry::proto::Progress* progress,
                                market::Market* market,
                                ProductionStepInfo* step_info) {
  if (progressed_.count(progress) > 0) {
    return false;
  }

  step_info->variants.clear();
  ++step_info->attempts_this_turn;
  GetStepInfo(production, *market, *field, *progress, step_info);

  unsigned int variant_index = step_info->variants.size();
  double max_profit = 0;
  double max_money = market->MaxMoney(wealth());
  double scale = 0;
  for (unsigned int idx = 0; idx < step_info->variants.size(); ++idx) {
    auto& variant_info = step_info->variants[idx];
    double cost_at_scale = variant_info.unit_cost * variant_info.possible_scale;
    double max_scale =
        std::min(variant_info.possible_scale, max_money / cost_at_scale);
    if (max_scale < 0.1) {
      continue;
    }
    double profit = market->GetPrice(production.ExpectedOutput(*progress));
    profit -= variant_info.unit_cost * max_scale;
    if (profit > max_profit) {
      max_profit = profit;
      variant_index = idx;
      scale = max_scale;
    }
  }

  if (variant_index >= step_info->variants.size()) {
    return false;
  }

  progress->set_scaling(scale);
  auto required = production.RequiredConsumables(*progress, variant_index);
  for (const auto& good : required.quantities()) {
    double amount_to_buy =
        good.second - market::GetAmount(wealth(), good.first);
    if (amount_to_buy > 0) {
      double bought =
          market->TryToBuy(good.first, amount_to_buy, mutable_wealth());
      if (bought < amount_to_buy) {
        // This should never happen.
        return false;
      }
    }
  }

  production.PerformStep(field->fixed_capital(), 0.0, variant_index,
                         mutable_wealth(), field->mutable_resources(),
                         mutable_wealth(), progress);
  progressed_.insert(progress);
  return true;
}

bool PopUnit::Produce(const ProductionMap& chains,
                      const std::vector<geography::proto::Field*>& fields,
                      market::Market* market) {
  bool any_progress = false;
  for (auto* field : fields) {
    if (field->has_production()) {
      auto* progress = field->mutable_production();
      const auto production = chains.find(progress->name());
      if (production == chains.end()) {
        // TODO: Error here!
        continue;
      }
      ProductionStepInfo step_info;
      if (TryProductionStep(*production->second, field, progress, market, &step_info)) {
        any_progress = true;
      }
    } else {
      // Do something else.
    }
  }
  return any_progress;
}

uint64 PopUnit::NewPopId() { return ++unused_pop_id; }

} // namespace population
