#ifndef AI_UNIT_AI_H
#define AI_UNIT_AI_H

#include "units/unit.h"
#include "actions/proto/strategy.pb.h"
#include "actions/proto/plan.pb.h"

namespace ai {

class UnitAi {
public:
  virtual void MakePlan(const actions::proto::Strategy& strategy,
                        actions::proto::Plan*) const = 0;
};

actions::proto::Plan MakePlan(const actions::proto::Strategy& strategy);

} // namespace ai

#endif
