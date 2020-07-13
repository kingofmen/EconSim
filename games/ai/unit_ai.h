#ifndef GAMES_AI_UNIT_AI_H
#define GAMES_AI_UNIT_AI_H

#include "games/actions/proto/strategy.pb.h"
#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"
#include "util/status/status.h"

namespace ai {

class UnitAi {
public:
  virtual util::Status AddStepsToPlan(const units::Unit& unit,
                                      const actions::proto::Strategy& strategy,
                                      actions::proto::Plan*) = 0;
};

} // namespace ai

#endif
