#ifndef AI_PLANNER_H
#define AI_PLANNER_H

#include "actions/proto/strategy.pb.h"
#include "actions/proto/plan.pb.h"
#include "games/ai/unit_ai.h"
#include "units/unit.h"
#include "util/status/status.h"

namespace ai {

util::Status MakePlan(const units::Unit& unit,
                      const actions::proto::Strategy& strategy,
                      actions::proto::Plan* plan);

util::Status RegisterPlanner(const actions::proto::Strategy& strategy,
                             UnitAi* planner);


} // namespace ai

#endif
