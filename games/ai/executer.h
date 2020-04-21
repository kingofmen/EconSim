#ifndef GAMES_AI_EXECUTER_H
#define GAMES_AI_EXECUTER_H

#include <string>

#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/status/status.h"

namespace ai {

typedef std::function<util::Status(const actions::proto::Step&, units::Unit*)>
    StepExecutor;

typedef std::function<uint64(const actions::proto::Step&, units::Unit*)>
    CostCalculator;

// Default CostCalculator that always returns 0.
uint64 ZeroCost(const actions::proto::Step&, units::Unit*);

void RegisterExecutor(const std::string& key, StepExecutor exe);
void RegisterExecutor(actions::proto::AtomicAction action, StepExecutor exe);

// Executes the first step of plan; returns a status showing either success, or
// what the problem was.
util::Status ExecuteStep(const actions::proto::Plan& plan, units::Unit* unit,
                         CostCalculator cost_u = ZeroCost);

// Deletes the first step of plan.
void DeleteStep(actions::proto::Plan* plan);


} // namespace ai

#endif
