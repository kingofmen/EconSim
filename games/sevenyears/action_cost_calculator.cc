#include "games/sevenyears/action_cost_calculator.h"

#include "util/arithmetic/microunits.h"

namespace sevenyears {

uint64 ActionCostCalculator::operator()(const actions::proto::Step& step,
                                        units::Unit* unit) {
  return micro::kOneInU;
}

} // namespace sevenyears
