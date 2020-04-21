#include "games/ai/executer.h"

#include <unordered_map>

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/ai/impl/executor_impl.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/status/status.h"

namespace ai {

std::unordered_map<actions::proto::AtomicAction, StepExecutor>
    execution_action_map = {
        {actions::proto::AA_MOVE, ai::impl::MoveUnit},
        {actions::proto::AA_BUY, ai::impl::BuyOrSell},
        {actions::proto::AA_SELL, ai::impl::BuyOrSell},
        {actions::proto::AA_TURN_AROUND, ai::impl::TurnAround},
        {actions::proto::AA_SWITCH_STATE, ai::impl::SwitchState},
};

std::unordered_map<std::string, StepExecutor> execution_key_map = {};

void RegisterExecutor(const std::string& key, StepExecutor exe) {
  execution_key_map[key] = exe;
}

void RegisterExecutor(actions::proto::AtomicAction action, StepExecutor exe) {
  execution_action_map[action] = exe;
}

uint64 ZeroCost(const actions::proto::Step&, units::Unit*) {
  return 0;
}

util::Status ExecuteStep(const actions::proto::Plan& plan, units::Unit* unit,
                         CostCalculator cost_u) {
  if (plan.steps_size() == 0) {
    return util::InvalidArgumentError("No steps in plan");
  }
  if (unit == nullptr) {
    return util::InvalidArgumentError("Null unit");
  }
  const auto& step = plan.steps(0);
  auto used_u = cost_u(step, unit);
  if (used_u > unit->action_points_u()) {
    return util::FailedPreconditionError("Not enough action points");
  }
  unit->use_action_points(used_u);
  switch (step.trigger_case()) {
    case actions::proto::Step::kKey:
      if (execution_key_map.find(step.key()) != execution_key_map.end()) {
        return execution_key_map.at(step.key())(step, unit);
      }
      return util::NotImplementedError(
          absl::Substitute("Executor for key $0 not implemented", step.key()));
    case actions::proto::Step::kAction:
      if (execution_action_map.find(step.action()) == execution_action_map.end()) {
        return util::NotImplementedError(absl::Substitute(
            "Executor for action $0 not implemented", step.action()));
      }
      return execution_action_map[step.action()](step, unit);
    case actions::proto::Step::TRIGGER_NOT_SET:
    default:
      return util::InvalidArgumentError("Plan step has neither action or key");
      break;
  }

  return util::NotImplementedError("This error message should be unreachable.");
}

void DeleteStep(actions::proto::Plan* plan) {
  if (plan->steps_size() == 1) {
    plan->clear_steps();
  } else if (plan->steps_size() > 1) {
    plan->mutable_steps()->DeleteSubrange(0, 1);
  }
}

} // namespace ai
