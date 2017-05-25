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

bool PopUnit::TryProductionStep(const industry::Production& production,
                                geography::proto::Field* field,
                                industry::proto::Progress* progress,
                                market::Market* market) {
  if (progressed_.count(progress) > 0) {
    return false;
  }

  const auto& step = production.steps(progress->step());
  int variant_index = -1;
  double least_cost = std::numeric_limits<double>::max();
  for (int index = 0; index < step.variants_size(); ++index) {
    const auto& input = step.variants(index);
    if (!(field->fixed_capital() > input.fixed_capital())) {
      continue;
    }
    if (!(field->resources() > input.raw_materials())) {
      continue;
    }

    double cost = 0;
    bool possible = true;
    auto required = input.consumables() + input.movable_capital();
    for (const auto& good : required.quantities()) {
      cost += market->GetPrice(good.first) * good.second;
      if (market->AvailableImmediately(good.first) <
          good.second - market::GetAmount(wealth(), good.first)) {
        possible = false;
        break;
      }
    }
    if (!possible) {
      continue;
    }
    if (cost < least_cost) {
      variant_index = index;
      least_cost = cost;
    }
  }

  if (variant_index == -1) {
    return false;
  }

  if (least_cost > market->MaxMoney(wealth())) {
    return false;
  }

  const auto& input = step.variants(variant_index);
  auto required = input.consumables() + input.movable_capital();
  for (const auto& good : required.quantities()) {
    double amount_to_buy =
        good.second - market::GetAmount(wealth(), good.first);
    if (amount_to_buy > 0) {
      market->TryToBuy(good.first, amount_to_buy, mutable_wealth());
    }
  }

  production.PerformStep(field->fixed_capital(), 0.0, variant_index,
                         mutable_wealth(), field->mutable_resources(),
                         mutable_wealth(), progress);
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
      if (TryProductionStep(*production->second, field, progress, market)) {
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
