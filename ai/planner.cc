#include "ai/planner.h"

#include <unordered_map>

#include "ai/unit_ai.h"
#include "ai/impl/unit_ai_impl.h"

namespace ai {

std::unordered_map<actions::proto::Strategy::StrategyCase, UnitAi*> unit_ai_map = {
  {actions::proto::Strategy::kShuttleTrade, new impl::ShuttleTrader()},
  {actions::proto::Strategy::kSevenYearsMerchant, new impl::SevenYearsMerchant()},
};

actions::proto::Plan MakePlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy) {
  actions::proto::Plan plan;
  if (unit_ai_map.find(strategy.strategy_case()) == unit_ai_map.end()) {
    // TODO: This is an error, handle it.
    return plan;
  }

  unit_ai_map[strategy.strategy_case()]->AddStepsToPlan(unit, strategy, &plan);
  return plan;
}

} // namespace ai
