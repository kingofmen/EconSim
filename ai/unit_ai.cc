#include "ai/unit_ai.h"

#include <unordered_map>

namespace ai {

std::unordered_map<actions::proto::Strategy::StrategyCase, UnitAi*> unit_ai_map;

actions::proto::Plan MakePlan(const actions::proto::Strategy& strategy) {
  actions::proto::Plan plan;
  if (unit_ai_map.find(strategy.strategy_case()) == unit_ai_map.end()) {
    return plan;
  }
  unit_ai_map[strategy.strategy_case()]->MakePlan(strategy, &plan);
  return plan;
}

} // namespace ai
