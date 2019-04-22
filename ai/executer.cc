#include "ai/executer.h"

#include <unordered_map>

#include "actions/proto/plan.pb.h"
#include "ai/impl/executor_impl.h"

namespace ai {

typedef std::function<bool(const actions::proto::Step&, units::Unit*)>
    StepExecutor;

std::unordered_map<actions::proto::AtomicAction, StepExecutor> execution_map = {
    {actions::proto::AA_MOVE, ai::impl::MoveUnit},
    {actions::proto::AA_BUY, ai::impl::BuyOrSell},
    {actions::proto::AA_SELL, ai::impl::BuyOrSell},
};

bool ExecutePlan(units::Unit* unit, actions::proto::Plan* plan) {
  int stepsTaken = 0;
  for (const auto& step : plan->steps()) {
    if (execution_map.find(step.action()) != execution_map.end()) {
      execution_map[step.action()](step, unit);
    }
    stepsTaken++;
  }
  if (stepsTaken >= plan->steps_size()) {
    plan->clear_steps();
  } else if (stepsTaken > 0) {
    plan->mutable_steps()->DeleteSubrange(0, stepsTaken);
  }

  return false;
}

} // namespace ai
