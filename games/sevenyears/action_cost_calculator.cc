#include "games/sevenyears/action_cost_calculator.h"

#include "util/arithmetic/microunits.h"

namespace sevenyears {

namespace {

uint64 cost(const actions::proto::AtomicAction action, units::Unit* unit) {
  switch (action) {
    case actions::proto::AA_MOVE:
      return micro::kOneInU;
    case actions::proto::AA_TURN_AROUND:
      return 0;
  }
  return micro::kOneInU;
}

uint64 cost(const std::string& action, units::Unit* unit) {
  return micro::kOneInU;
}

}  // namespace

uint64 ActionCostCalculator::operator()(const actions::proto::Step& step,
                                        units::Unit* unit) {
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
