#ifndef AI_EXECUTER_H
#define AI_EXECUTER_H

#include "actions/proto/plan.pb.h"
#include "units/unit.h"

namespace ai {

// Attempts to execute steps of plan. Returns true if the unit is able
// to do anything else.
bool ExecutePlan(units::Unit* unit, actions::proto::Plan* plan);

} // namespace ai

#endif
