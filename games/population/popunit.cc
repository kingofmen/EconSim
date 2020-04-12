#include "games/population/popunit.h"

#include <cmath>
#include <limits>
#include <list>
#include <iostream>

#include "absl/algorithm/container.h"
#include "games/geography/proto/geography.pb.h"
#include "games/industry/industry.h"
#include "util/arithmetic/microunits.h"
#include "util/keywords/keywords.h"

namespace population {

using industry::proto::Production;

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

void BuyPackage(const proto::ConsumptionPackage& package, int size,
                market::proto::Container* resources, market::Market* market) {
  auto needed = TotalNeeded(package, size);
  needed -= *resources;
  market->BuyBasket(needed, resources);
}

} // namespace

std::unordered_map<uint64, PopUnit*> PopUnit::id_to_pop_map_;

PopUnit::PopUnit() : packages_ordered_(0) {
  proto_.set_pop_id(NewPopId());
  for (int i = 0; i < kNumAgeGroups; ++i) {
    proto_.add_males(0);
    proto_.add_women(0);
  }
  id_to_pop_map_[proto_.pop_id()] = this;
}

PopUnit::PopUnit(const proto::PopUnit& proto) : proto_(proto) {
  if (id_to_pop_map_[proto_.pop_id()] != nullptr) {
    // TODO: Error here.
  }
  id_to_pop_map_[proto_.pop_id()] = this;
}

void PopUnit::AutoProduce(
    const std::vector<proto::AutoProduction>& production,
    market::Market* market) {
  int bestIndex = -1;
  market::Measure best_price = 0;
  for (int idx = 0; idx < production.size(); ++idx) {
    const auto& p = production[idx];
    if (!(proto_.tags() > p.required_tags())) {
      continue;
    }
    auto curr_price = market->GetPriceU(p.output());
    if (curr_price < best_price) {
      continue;
    }
    best_price = curr_price;
    bestIndex = idx;
  }
  if (bestIndex < 0) {
    return;
  }
  *mutable_wealth() += production[bestIndex].output() * GetSize();
  SellSurplus(market);
}

void PopUnit::BirthAndDeath() {}

const proto::ConsumptionPackage*
PopUnit::CheapestPackage(const proto::ConsumptionLevel& level,
                         const market::Market& market,
                         const proto::ConsumptionPackage*& cheapest) const {
  const proto::ConsumptionPackage* best_package = nullptr;
  market::Measure best_available_u = std::numeric_limits<market::Measure>::max();
  market::Measure cheapest_price_u = std::numeric_limits<market::Measure>::max();
  const int size = GetSize();
  auto max_money = market.MaxMoney(proto_.wealth());
  for (const auto& package : level.packages()) {
    if (!(proto_.tags() > package.required_tags())) {
      continue;
    }

    auto needed = TotalNeeded(package, size);
    if (proto_.wealth() > needed) {
      auto curr_price_u = market.GetPriceU(package.consumed());
      if (curr_price_u < best_available_u) {
        best_package = &package;
        best_available_u = curr_price_u;
      }
      if (curr_price_u < cheapest_price_u) {
        cheapest = &package;
        cheapest_price_u = curr_price_u;
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
    if (curr_price_u < best_available_u && can_buy) {
      best_package = &package;
      best_available_u = curr_price_u;
    }
    if (curr_price_u < cheapest_price_u) {
      cheapest = &package;
      cheapest_price_u = curr_price_u;
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
  *resources -= best_package->capital() * size;
  used_capital_ += best_package->capital() * size;

  *proto_.mutable_tags() += best_package->tags();
  *proto_.mutable_tags() += level.tags();
  return true;
}

void PopUnit::EndTurn(const market::proto::Container& decay_rates_u) {
  *mutable_wealth() << used_capital_;
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

void PopUnit::StartTurn(const std::vector<proto::ConsumptionLevel>& levels,
                        market::Market* market) {
  packages_ordered_ = 0;
  subsistence_need_.Clear();
  market::Measure found = 0;
  const proto::ConsumptionPackage* cheapest = nullptr;
  for (const auto& level : levels) {
    auto amount = market::GetAmount(level.tags(), keywords::kSubsistenceTag);
    if (amount <= 0) {
      continue;
    }
    const proto::ConsumptionPackage* best_package =
        CheapestPackage(level, *market, cheapest);
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

void PopUnit::ReturnCapital(market::proto::Container* caps) {
  used_capital_ << *caps;
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
