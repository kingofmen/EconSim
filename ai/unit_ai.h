#ifndef AI_UNIT_AI_H
#define AI_UNIT_AI_H

#include "actions/proto/strategy.pb.h"
#include "actions/proto/plan.pb.h"
#include "units/unit.h"

namespace ai {

class UnitAi {
public:
  virtual void AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan*) const = 0;
};

} // namespace ai

#endif
