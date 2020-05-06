#ifndef GAMES_SEVENYEARS_ACTION_COST_CALCULATOR_H
#define GAMES_SEVENYEARS_ACTION_COST_CALCULATOR_H

#include "games/actions/proto/plan.pb.h"
#include "games/sevenyears/interfaces.h"
#include "games/units/unit.h"
#include "util/status/status.h"

namespace sevenyears {

class SevenYears;

class ActionCostCalculator {
public:
  ActionCostCalculator(const sevenyears::SevenYearsState* seven)
      : game_(seven) {}

  // To satisfy CostCalculator typedef.
  uint64 operator()(const actions::proto::Step& step, units::Unit* unit);

private:
  const sevenyears::SevenYearsState* game_;
};

}  // namespace sevenyears

#endif
