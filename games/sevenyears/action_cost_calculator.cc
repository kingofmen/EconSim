#include "games/sevenyears/action_cost_calculator.h"

#include "util/arithmetic/microunits.h"

namespace sevenyears {

namespace {

ai::ActionCost cost(const actions::proto::AtomicAction action,
                    const units::Unit& unit) {
  switch (action) {
    case actions::proto::AA_MOVE:
      return ai::OneCost();
    case actions::proto::AA_TURN_AROUND:
      return ai::ZeroCost();
  }
  return ai::OneCost();
}

ai::ActionCost cost(const std::string& action, const units::Unit& unit) {
  return ai::OneCost();
}

}  // namespace

ai::ActionCost ActionCostCalculator::
operator()(const actions::proto::Step& step, const units::Unit& unit) {
  switch (step.trigger_case()) {
    case actions::proto::Step::kKey:
      return cost(step.key(), unit);
    case actions::proto::Step::kAction:
      return cost(step.action(), unit);
    case actions::proto::Step::TRIGGER_NOT_SET:
    default:
      return ai::OneCost();
  }
  return ai::OneCost();
}

} // namespace sevenyears
