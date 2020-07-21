#ifndef GAMES_AI_IMPL_UNIT_AI_IMPL_H
#define GAMES_AI_IMPL_UNIT_AI_IMPL_H

#include <vector>

#include "games/actions/proto/strategy.pb.h"
#include "games/actions/proto/plan.pb.h"
#include "games/ai/unit_ai.h"
#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/status/status.h"

namespace ai {
namespace impl {

typedef std::function<micro::Measure(const geography::Connection&)>
    CostFunction;

typedef std::function<micro::Measure(const util::proto::ObjectId&,
                                     const util::proto::ObjectId&)>
    Heuristic;

// Cost function returning the length of the connection.
micro::Measure ShortestDistance(const geography::Connection& conn);

// Default heuristic that doesn't actually heurise.
micro::Measure ZeroHeuristic(const util::proto::ObjectId& cand_id,
                             const util::proto::ObjectId& target_id);

// Returns the total distance of all the identified connections as
// calculated by the given cost function.
micro::Measure CalculateDistance(const std::vector<uint64>& path,
                                 const CostFunction& cost_function);

// Fills path with the lowest-cost steps (using connection IDs) from source to
// destination.
util::Status FindPath(const geography::proto::Location& source,
                      const CostFunction& cost_function,
                      const Heuristic& heuristic,
                      const util::proto::ObjectId& target_id,
                      std::vector<uint64>* path);

// Adds the steps in path to plan.
void PlanPath(const geography::proto::Location& source,
              const std::vector<uint64>& path, actions::proto::Plan* plan);

// Adds to plan steps for traversing the connections between unit's current
// location and the provided target area.
util::Status FindPath(const units::Unit& unit,
                      const CostFunction& cost_function,
                      const Heuristic& heuristic,
                      const util::proto::ObjectId& target_id,
                      actions::proto::Plan* plan);

class ShuttleTrader : public ai::UnitAi {
public:
  util::Status AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan* plan) override;

private:
};

} // namespace impl
} // namespace ai

#endif
