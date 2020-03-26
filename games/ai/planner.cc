#include "games/ai/planner.h"

#include <unordered_map>

#include "absl/strings/substitute.h"
#include "games/ai/unit_ai.h"
#include "games/ai/impl/unit_ai_impl.h"
#include "util/status/status.h"

namespace ai {

std::unordered_map<actions::proto::Strategy::StrategyCase, UnitAi*> unit_ai_map = {
  {actions::proto::Strategy::kShuttleTrade, new impl::ShuttleTrader()},
  {actions::proto::Strategy::kSevenYearsMerchant, new impl::SevenYearsMerchant()},
};

util::Status MakePlan(const units::Unit& unit,
                      const actions::proto::Strategy& strategy,
                      actions::proto::Plan* plan) {
  if (unit_ai_map.find(strategy.strategy_case()) == unit_ai_map.end()) {
    return util::NotFoundError(
        absl::Substitute("Unknown strategy case $0 in $1",
                         strategy.strategy_case(), strategy.DebugString()));
  }

  return unit_ai_map[strategy.strategy_case()]->AddStepsToPlan(unit, strategy, plan);
}

util::Status RegisterPlanner(const actions::proto::Strategy& strategy,
                             UnitAi* planner) {
  if (planner == NULL) {
    return util::InvalidArgumentError(
        absl::Substitute("Null planner passed for $0", strategy.DebugString()));
  }
  unit_ai_map[strategy.strategy_case()] = planner;
  return util::OkStatus();
}

} // namespace ai
