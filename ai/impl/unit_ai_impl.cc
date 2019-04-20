#include "unit_ai_impl.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "market/goods_utils.h"
#include "geography/connection.h"
#include "units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"

#include <iostream>
namespace ai {
namespace impl {
namespace {

// Adds to plan steps for traversing the connections between unit's current
// location and the provided target area.
void FindPath(const units::Unit& unit, uint64 target_id,
              actions::proto::Plan* plan) {
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

  // Placeholder cost and heuristic functions - user should specify.
  auto costFunction = [](const geography::Connection* conn) -> market::Measure {
    return conn->length_u();
  };
  auto heuristic = [](uint64 cand_id, uint64 target_id) -> market::Measure {
    return 0;
  };

  // TODO: Include a give-up condition and handler.
  while(open.size() > 0) {
    const std::unordered_set<geography::Connection*>& conns =
        geography::Connection::ByEndpoint(least_cost_id);
    for (const auto* conn : conns) {
      uint64 other_side_id = conn->OtherSide(least_cost_id);
      market::Measure real_cost_u = least_cost_u + costFunction(conn);
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
      least_cost_u = visited[cand_id].cost_u;
      least_cost_id = cand_id;
    }
    least_cost_u = visited[least_cost_id].cost_u;
    if (least_cost_id == target_id) {
      break;
    }
  }

  if (visited.find(target_id) == visited.end()) {
    std::cout << __FILE__ << ":" << __LINE__ << "\n";
    // Didn't find a path.
    return;
  }

  std::vector<uint64> path;
  uint64 current_id = target_id;
  while (current_id != start_id) {
    std::cout << __FILE__ << ":" << __LINE__ << "\n";
    path.push_back(current_id);
    current_id = visited[current_id].previous_id;
  }

  while (!path.empty()) {
    std::cout << __FILE__ << ":" << __LINE__ << "\n";
    uint64 id = path.back();
    path.pop_back();
    auto* step = plan->add_steps();
    step->set_action(actions::proto::AA_MOVE);
    step->set_connection_id(visited[id].conn_id);
  }
}

// Adds transit, selling, and buying steps to plan; returns true if the state
// should flip.
bool GoBuySell(const units::Unit& unit, uint64 target_id, const std::string& buy,
               const std::string& sell, actions::proto::Plan* plan) {
  std::cout << __FILE__ << ":" << __LINE__ << "\n";
  if (unit.location().source_area_id() != target_id) {
    std::cout << __FILE__ << ":" << __LINE__ << "\n";
    FindPath(unit, target_id, plan);
    return false;
  }
  std::cout << __FILE__ << ":" << __LINE__ << "\n";

  if (market::GetAmount(unit.resources(), sell) > 0) {
    auto* step = plan->add_steps();
    std::cout << __FILE__ << ":" << __LINE__ << "\n";
    step->set_action(actions::proto::AA_SELL);
    step->set_good(sell);
  }
  // TODO: Take actual cargo capacity into account here.
  if (market::GetAmount(unit.resources(), buy) < micro::kOneInU) {
    std::cout << __FILE__ << ":" << __LINE__ << "\n";
    auto* step = plan->add_steps();
    step->set_action(actions::proto::AA_BUY);
    step->set_good(buy);
    return false;
  }
    std::cout << __FILE__ << ":" << __LINE__ << "\n";

  return true;
}

}

void ShuttleTrader::AddStepsToPlan(const units::Unit& unit,
                                   actions::proto::Strategy* strategy,
                                   actions::proto::Plan* plan) const {
  if (!strategy->has_shuttle_trade()) {
    std::cout << __FILE__ << ":" << __LINE__ << "\n";
    // TODO: Handle this as error? It seems to indicate something unexpected
    // anyway.
    return;
  }

  const actions::proto::ShuttleTrade& info = strategy->shuttle_trade();
  switch (info.state()) {
    case actions::proto::ShuttleTrade::STS_BUY_A:
    std::cout << __FILE__ << ":" << __LINE__ << "\n";
    if (GoBuySell(unit, info.area_a_id(), info.good_a(), info.good_z(), plan)) {
        strategy->mutable_shuttle_trade()->set_state(
            actions::proto::ShuttleTrade::STS_BUY_Z);
      }
      break;
    case actions::proto::ShuttleTrade::STS_BUY_Z:
      if (GoBuySell(unit, info.area_z_id(), info.good_z(), info.good_a(), plan)) {
        strategy->mutable_shuttle_trade()->set_state(
            actions::proto::ShuttleTrade::STS_BUY_A);
      }
      break;
    default:
      break;
  }
}

} // namespace impl
} // namespace ai
