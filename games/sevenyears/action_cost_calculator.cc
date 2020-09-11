#include "games/sevenyears/action_cost_calculator.h"

#include "util/arithmetic/microunits.h"

namespace sevenyears {

namespace {

micro::uMeasure cost(const actions::proto::AtomicAction action,
                     const units::Unit& unit) {
  switch (action) {
    case actions::proto::AA_MOVE:
      return micro::kOneInU;
    case actions::proto::AA_TURN_AROUND:
      return 0;
  }
  return micro::kOneInU;
}

micro::uMeasure cost(const std::string& action, const units::Unit& unit) {
  return micro::kOneInU;
}

}  // namespace

micro::uMeasure ActionCostCalculator::
operator()(const actions::proto::Step& step, const units::Unit& unit) {
  switch (step.trigger_case()) {
    case actions::proto::Step::kKey:
      return cost(step.key(), unit);
    case actions::proto::Step::kAction:
      return cost(step.action(), unit);
    case actions::proto::Step::TRIGGER_NOT_SET:
    default:
      return micro::kOneInU;
  }
  return micro::kOneInU;
}

} // namespace sevenyears
