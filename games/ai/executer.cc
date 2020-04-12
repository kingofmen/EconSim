#include "games/ai/executer.h"

#include <unordered_map>

#include "games/actions/proto/plan.pb.h"
#include "games/ai/impl/executor_impl.h"

namespace ai {

typedef std::function<bool(const actions::proto::Step&, units::Unit*)>
    StepExecutor;

std::unordered_map<actions::proto::AtomicAction, StepExecutor> execution_map = {
    {actions::proto::AA_MOVE, ai::impl::MoveUnit},
    {actions::proto::AA_BUY, ai::impl::BuyOrSell},
    {actions::proto::AA_SELL, ai::impl::BuyOrSell},
    {actions::proto::AA_SWITCH_STATE, ai::impl::SwitchState},
};

bool ExecuteStep(const actions::proto::Plan& plan, units::Unit* unit) {
  if (plan.steps_size() == 0) {
    return false;
  }
  const auto& step = plan.steps(0);
  if (execution_map.find(step.action()) == execution_map.end()) {
    return false;
  }
  execution_map[step.action()](step, unit);
  return true;
}

void DeleteStep(actions::proto::Plan* plan) {
  if (plan->steps_size() == 1) {
    plan->clear_steps();
  } else if (plan->steps_size() > 1) {
    plan->mutable_steps()->DeleteSubrange(0, 1);
  }
}

} // namespace ai
