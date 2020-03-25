#include "unit_ai_impl.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "market/goods_utils.h"
#include "geography/connection.h"
#include "units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"

typedef std::function<market::Measure(const geography::Connection&)>
    CostFunction;

typedef std::function<market::Measure(uint64, uint64)> Heuristic;

namespace ai {
namespace impl {
namespace {

// Cost function returning the length of the connection.
market::Measure ShortestDistance(const geography::Connection& conn) {
  return conn.length_u();
}

// Default heuristic that doesn't actually heurise.
market::Measure ZeroHeuristic(uint64 cand_id, uint64 target_id) {
  return 0;
}

// Adds to plan steps for traversing the connections between unit's current
// location and the provided target area.
void FindPath(const units::Unit& unit, const CostFunction& cost_function,
              const Heuristic& heuristic, 
              uint64 target_id, actions::proto::Plan* plan) {
  std::unordered_set<uint64> open;
  struct Node {
    uint64 previous_id;
    uint64 conn_id;
    market::Measure cost_u;
    market::Measure heuristic_u;
  };
  std::unordered_map<uint64, Node> visited;

  uint64 start_id = unit.location().source_area_id();
  uint64 least_cost_id = start_id;
  market::Measure least_cost_u = 0;
  open.insert(least_cost_id);
  visited.insert({least_cost_id, {0, 0, least_cost_u}});
  // No closed set as we cannot guarantee the heuristic is consistent, and
  // besides it would interact badly with the possibility of multiple edges
  // between two nodes.

  // TODO: Include a give-up condition and handler.
  while(open.size() > 0) {
    const std::unordered_set<geography::Connection*>& conns =
        geography::Connection::ByEndpoint(least_cost_id);
    for (const auto* conn : conns) {
      uint64 other_side_id = conn->OtherSide(least_cost_id);
      market::Measure real_cost_u = least_cost_u + cost_function(*conn);
      const auto& existing = visited.find(other_side_id);
      if (existing == visited.end()) {
        visited.insert({other_side_id,
                        {least_cost_id, conn->ID(), real_cost_u,
                         heuristic(other_side_id, target_id)}});
        open.insert(other_side_id);
      } else if (real_cost_u < existing->second.cost_u) {
        existing->second.conn_id = conn->ID();
        existing->second.cost_u = real_cost_u;
        open.insert(other_side_id);
      }
    }

    open.erase(least_cost_id);
    least_cost_u = micro::kMaxU;
    for (auto cand_id : open) {
      // TODO: Consider the tiebreak here.
      if (visited[cand_id].cost_u + visited[cand_id].heuristic_u >= least_cost_u) {
        continue;
      }
      least_cost_u = visited[cand_id].cost_u + visited[cand_id].heuristic_u;
      least_cost_id = cand_id;
    }
    if (least_cost_id == target_id) {
      break;
    }
    least_cost_u = visited[least_cost_id].cost_u;
  }

  if (visited.find(target_id) == visited.end()) {
    // Didn't find a path.
    return;
  }

  std::vector<uint64> path;
  uint64 current_id = target_id;
  while (current_id != start_id) {
    path.push_back(current_id);
    current_id = visited[current_id].previous_id;
  }

  while (!path.empty()) {
    uint64 id = path.back();
    path.pop_back();
    auto* step = plan->add_steps();
    step->set_action(actions::proto::AA_MOVE);
    step->set_connection_id(visited[id].conn_id);
  }
}

// Adds transit, selling, buying, and flipping steps to plan.
void GoBuySell(const units::Unit& unit, uint64 target_id, const std::string& buy,
               const std::string& sell, actions::proto::Plan* plan) {
  if (unit.location().source_area_id() != target_id) {
    FindPath(unit, ShortestDistance, ZeroHeuristic, target_id, plan);
  }

  auto* step = plan->add_steps();
  step->set_action(actions::proto::AA_SELL);
  step->set_good(sell);

  step = plan->add_steps();
  step->set_action(actions::proto::AA_BUY);
  step->set_good(buy);

  step = plan->add_steps();
  step->set_action(actions::proto::AA_SWITCH_STATE);
}

}

void ShuttleTrader::AddStepsToPlan(const units::Unit& unit,
                                   const actions::proto::Strategy& strategy,
                                   actions::proto::Plan* plan) const {
  if (!strategy.has_shuttle_trade()) {
    // TODO: Handle this as error? It seems to indicate something unexpected
    // anyway.
    return;
  }
  const actions::proto::ShuttleTrade& info = strategy.shuttle_trade();
  switch (info.state()) {
    case actions::proto::ShuttleTrade::STS_BUY_A:
      GoBuySell(unit, info.area_a_id(), info.good_a(), info.good_z(), plan);
      break;
    case actions::proto::ShuttleTrade::STS_BUY_Z:
      GoBuySell(unit, info.area_z_id(), info.good_z(), info.good_a(), plan);
      break;
    default:
      break;
  }
}

void SevenYearsMerchant::AddStepsToPlan(
    const units::Unit& unit, const actions::proto::Strategy& strategy,
    actions::proto::Plan* plan) const {
  if (!strategy.has_seven_years_merchant()) {
    // TODO: Error.
    return;
  }

  return;
}

} // namespace impl
} // namespace ai
