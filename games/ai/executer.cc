#include "games/ai/executer.h"

#include <unordered_map>

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/ai/impl/executor_impl.h"
#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/status/status.h"

namespace ai {
namespace {

std::unordered_map<actions::proto::AtomicAction, StepExecutor>
    execution_action_map = {
        {actions::proto::AA_MOVE, ai::impl::MoveUnit},
        {actions::proto::AA_BUY, ai::impl::BuyOrSell},
        {actions::proto::AA_SELL, ai::impl::BuyOrSell},
        {actions::proto::AA_TURN_AROUND, ai::impl::TurnAround},
        {actions::proto::AA_SWITCH_STATE, ai::impl::SwitchState},
};
std::unordered_map<actions::proto::AtomicAction, CostCalculator>
    action_cost_map = {};

std::unordered_map<std::string, StepExecutor> execution_key_map = {};
std::unordered_map<std::string, CostCalculator> key_cost_map = {};

CostCalculator default_cost = nullptr;

util::Status executeKey(const std::string& key,
                        const actions::proto::Step& step, units::Unit* unit) {
  if (execution_key_map.find(key) != execution_key_map.end()) {
    return execution_key_map.at(key)(step, unit);
  }
  return util::NotImplementedError(
      absl::Substitute("Executor for key $0 not implemented", key));
}

util::Status executeAction(actions::proto::AtomicAction action,
                           const actions::proto::Step& step,
                           units::Unit* unit) {
  if (execution_action_map.find(action) == execution_action_map.end()) {
    return util::NotImplementedError(
        absl::Substitute("Executor for action $0 not implemented", action));
  }
  return execution_action_map[step.action()](step, unit);  
}

}  // namespace

ActionCost GetCost(const actions::proto::Step& step, const units::Unit& unit) {
  switch (step.trigger_case()) {
    case actions::proto::Step::kKey:
      if (key_cost_map.find(step.key()) != key_cost_map.end()) {
        return key_cost_map.at(step.key())(step, unit);
      }
    case actions::proto::Step::kAction:
      if (action_cost_map.find(step.action()) != action_cost_map.end()) {
        return action_cost_map.at(step.action())(step, unit);
      }
    default:
      break;
  }
  if (default_cost != nullptr) {
    return default_cost(step, unit);
  }
  return ZeroCost(step, unit);
}

void RegisterCost(const std::string& key, CostCalculator cost) {
  key_cost_map[key] = cost;
}

void RegisterCost(actions::proto::AtomicAction action, CostCalculator cost) {
  action_cost_map[action] = cost;
}

void RegisterDefaultCost(CostCalculator cost) {
  default_cost = cost;
}

void RegisterExecutor(const std::string& key, StepExecutor exe) {
  execution_key_map[key] = exe;
}

void RegisterExecutor(actions::proto::AtomicAction action, StepExecutor exe) {
  execution_action_map[action] = exe;
}

ActionCost ZeroCost() {
  return ActionCost(0, micro::kOneInU);
}

ActionCost OneCost() {
  return ActionCost(micro::kOneInU, micro::kOneInU);
}

ActionCost InfiniteCost() {
  return ActionCost(micro::kuMaxU, 0);
}

ActionCost ZeroCost(const actions::proto::Step&, const units::Unit&) {
  return ZeroCost();
}

ActionCost OneCost(const actions::proto::Step&, const units::Unit&) {
  return OneCost();
}

ActionCost InfiniteCost(const actions::proto::Step&, const units::Unit&) {
  return InfiniteCost();
}

ActionCost DefaultMoveCost(const actions::proto::Step& step, const units::Unit& unit) {
  if (step.trigger_case() != actions::proto::Step::kAction) {
    return InfiniteCost();
  }
  if (!step.has_connection_id()) {
    return InfiniteCost();
  }
  const geography::proto::Location& location = unit.location();
  const geography::Connection* connection = nullptr;
  if (location.has_connection_id()) {
    connection = geography::Connection::ById(location.connection_id());
  } else {
    connection = geography::Connection::ById(step.connection_id());
  }
  if (!connection) {
    DLOGF(Log::P_DEBUG, "  DefaultMoveCost could not find connection ID %d",
          location.has_connection_id() ? location.connection_id()
                                       : step.connection_id());
    return InfiniteCost();
  }

  micro::uMeasure progress_u = location.progress_u();
  micro::uMeasure distance_u = connection->length_u() - progress_u;
  micro::uMeasure speed_u = unit.speed_u(connection->type());

  if (distance_u >= speed_u) {
    // We won't finish this in one step.
    return ActionCost(std::min(micro::kOneInU, unit.action_points_u()),
                      micro::DivideU(speed_u, distance_u));
  }

  return ActionCost(micro::DivideU(distance_u, speed_u), micro::kOneInU);
}

util::Status ExecuteStep(const actions::proto::Plan& plan, units::Unit* unit) {
  if (plan.steps_size() == 0) {
    return util::InvalidArgumentError("No steps in plan");
  }
  if (unit == nullptr) {
    return util::InvalidArgumentError("Null unit");
  }
  const auto& step = plan.steps(0);
  const auto action = GetCost(step, *unit);
  if (action.cost_u > unit->action_points_u()) {
    return util::FailedPreconditionError("Not enough action points");
  }
  unit->use_action_points(action.cost_u);
  switch (step.trigger_case()) {
    case actions::proto::Step::kKey:
      return executeKey(step.key(), step, unit);
    case actions::proto::Step::kAction:
      return executeAction(step.action(), step, unit);
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
