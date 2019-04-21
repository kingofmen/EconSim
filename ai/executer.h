#ifndef AI_EXECUTER_H
#define AI_EXECUTER_H

#include "actions/proto/plan.pb.h"
#include "units/unit.h"

namespace ai {

void ExecutePlan(const actions::proto::Plan& plan, units::Unit* unit);

} // namespace ai

#endif
