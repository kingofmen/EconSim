#ifndef GAMES_AI_EXECUTER_H
#define GAMES_AI_EXECUTER_H

#include <string>

#include "games/ai/public/cost.h"
#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/status/status.h"

namespace ai {

typedef std::function<util::Status(const ActionCost&,
                                   const actions::proto::Step&, units::Unit*)>
    StepExecutor;

void RegisterExecutor(const std::string& key, StepExecutor exe);
void RegisterExecutor(actions::proto::AtomicAction action, StepExecutor exe);

void RegisterCost(const std::string& key, CostCalculator cost);
void RegisterCost(actions::proto::AtomicAction action, CostCalculator cost);
void RegisterDefaultCost(CostCalculator cost);

// Executes the first step of plan; returns a status showing either success, or
// what the problem was.
util::Status ExecuteStep(const actions::proto::Plan& plan, units::Unit* unit);

// Deletes the first step of plan.
void DeleteStep(actions::proto::Plan* plan);

// Returns the cost of the action for the unit, first looking for a specially
// registered CostCalculator for the action, then the default cost calculator,
// finally falling back to returning zero.
ActionCost GetCost(const actions::proto::Step& step,
                   const units::Unit& unit);

} // namespace ai

#endif
