#include "games/ai/executer.h"

#include <unordered_map>

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/ai/impl/executor_impl.h"
#include "util/status/status.h"

namespace ai {

std::unordered_map<actions::proto::AtomicAction, StepExecutor>
    execution_action_map = {
        {actions::proto::AA_MOVE, ai::impl::MoveUnit},
        {actions::proto::AA_BUY, ai::impl::BuyOrSell},
        {actions::proto::AA_SELL, ai::impl::BuyOrSell},
        {actions::proto::AA_SWITCH_STATE, ai::impl::SwitchState},
};

std::unordered_map<std::string, StepExecutor> execution_key_map = {};

void RegisterExecutor(const std::string& key, StepExecutor exe) {
  execution_key_map[key] = exe;
}

void RegisterExecutor(actions::proto::AtomicAction action, StepExecutor exe) {
  execution_action_map[action] = exe;
}

util::Status ExecuteStep(const actions::proto::Plan& plan, units::Unit* unit) {
  if (plan.steps_size() == 0) {
    return util::InvalidArgumentError("No steps in plan");
  }
  const auto& step = plan.steps(0);
  if (step.has_key()) {
    if (execution_key_map.find(step.key()) != execution_key_map.end()) {
      return execution_key_map.at(step.key())(step, unit);
    }
    return util::NotImplementedError(
        absl::Substitute("Executor for key $0 not implemented", step.key()));
  }

  if (!step.has_action()) {
    return util::InvalidArgumentError("Plan step has no action");
  }

  if (execution_action_map.find(step.action()) == execution_action_map.end()) {
    return util::NotImplementedError(absl::Substitute(
        "Executor for action $0 not implemented", step.action()));
  }

  return execution_action_map[step.action()](step, unit);
}

void DeleteStep(actions::proto::Plan* plan) {
  if (plan->steps_size() == 1) {
    plan->clear_steps();
  } else if (plan->steps_size() > 1) {
    plan->mutable_steps()->DeleteSubrange(0, 1);
  }
}

} // namespace ai
