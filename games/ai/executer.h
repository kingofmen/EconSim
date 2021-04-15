#ifndef GAMES_AI_EXECUTER_H
#define GAMES_AI_EXECUTER_H

#include <string>

#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/status/status.h"

namespace ai {

typedef micro::uMeasure ActionCost;

typedef std::function<util::Status(const actions::proto::Step&, units::Unit*)>
    StepExecutor;

typedef std::function<micro::uMeasure(const actions::proto::Step&, const units::Unit&)>
    CostCalculator;

// Default CostCalculator that always returns 0.
ActionCost ZeroCost(const actions::proto::Step& step,
                    const units::Unit& unit);
// Default CostCalculator that always returns 1.
ActionCost OneCost(const actions::proto::Step& step,
                        const units::Unit& unit);
// Variable cost for movement.
ActionCost DefaultMoveCost(const actions::proto::Step& step,
                           const units::Unit& unit);

// Returns the cost of the action for the unit, first looking for a specially
// registered CostCalculator for the action, then the default cost calculator,
// finally falling back to returning zero.
ActionCost GetCost(const actions::proto::Step& step,
                   const units::Unit& unit);

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
