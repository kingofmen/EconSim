#ifndef GAMES_AI_EXECUTER_H
#define GAMES_AI_EXECUTER_H

#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"

namespace ai {

// Executes the first step of plan; returns true if execution succeeded.
bool ExecuteStep(const actions::proto::Plan& plan, units::Unit* unit);

// Deletes the first step of plan.
void DeleteStep(actions::proto::Plan* plan);


} // namespace ai

#endif
