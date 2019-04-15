#ifndef AI_PLANNER_H
#define AI_PLANNER_H

#include "actions/proto/strategy.pb.h"
#include "actions/proto/plan.pb.h"
#include "units/unit.h"

namespace ai {

actions::proto::Plan MakePlan(const units::Unit& unit,
                              actions::proto::Strategy* strategy);

} // namespace ai

#endif
