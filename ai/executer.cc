#include "ai/executer.h"

#include <unordered_map>

#include "actions/proto/plan.pb.h"

namespace ai {

typedef std::function<bool(const actions::proto::Step&, units::Unit*)>
    StepExecutor;

std::unordered_map<actions::proto::AtomicAction, StepExecutor> execution_map =
    {};

void ExecutePlan(const actions::proto::Plan& plan, units::Unit* unit) {
  for (const auto& step : plan.steps()) {
    if (execution_map.find(step.action()) == execution_map.end()) {
      // TODO: Handle this.
      continue;
    }

    execution_map[step.action()](step, unit);
  }
}

} // namespace ai
