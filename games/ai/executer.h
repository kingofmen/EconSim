#ifndef GAMES_AI_EXECUTER_H
#define GAMES_AI_EXECUTER_H

#include <string>

#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"

namespace ai {

typedef std::function<bool(const actions::proto::Step&, units::Unit*)>
    StepExecutor;

void RegisterExecutor(const std::string& key, StepExecutor exe);
void RegisterExecutor(actions::proto::AtomicAction action, StepExecutor exe);

// Executes the first step of plan; returns true if execution succeeded.
bool ExecuteStep(const actions::proto::Plan& plan, units::Unit* unit);

// Deletes the first step of plan.
void DeleteStep(actions::proto::Plan* plan);


} // namespace ai

#endif
