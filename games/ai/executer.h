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
// Default CostCalculator that always returns 1.
uint64 OneCost(const actions::proto::Step&, units::Unit*);

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


} // namespace ai

#endif
