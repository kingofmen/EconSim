#ifndef GAMES_AI_PUBLIC_COST_H
#define GAMES_AI_PUBLIC_COST_H

#include <string>

#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"

namespace ai {

struct ActionCost {
  ActionCost(micro::uMeasure c_u, micro::uMeasure f_u)
      : cost_u(c_u), fraction_u(f_u) {}

  // The amount to be subtracted from the actor's reserve.
  micro::uMeasure cost_u;

  // The completion fraction of the task.
  micro::uMeasure fraction_u;
};

typedef std::function<ActionCost(const actions::proto::Step&, const units::Unit&)>
    CostCalculator;

// Default CostCalculator that always returns 0.
ActionCost ZeroCost(const actions::proto::Step& step,
                    const units::Unit& unit);
ActionCost ZeroCost();
// Default CostCalculator that always returns 1.
ActionCost OneCost(const actions::proto::Step& step,
                   const units::Unit& unit);
ActionCost OneCost();
// Default CostCalculator that always returns 0
// completion, maximum possible cost.
ActionCost InfiniteCost(const actions::proto::Step& step,
                        const units::Unit& unit);
ActionCost InfiniteCost();
// Variable cost for movement.
ActionCost DefaultMoveCost(const actions::proto::Step& step,
                           const units::Unit& unit);

} // namespace ai

#endif
