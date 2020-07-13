#include "games/ai/impl/unit_ai_impl.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "absl/strings/substitute.h"
#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.h"

namespace ai {
namespace impl {
namespace {

// Adds transit, selling, buying, and flipping steps to plan.
void GoBuySell(const units::Unit& unit, const util::proto::ObjectId& target_id,
               const std::string& buy, const std::string& sell,
               actions::proto::Plan* plan) {
  if (unit.location().a_area_id() != target_id) {
    auto status =
        FindPath(unit, ShortestDistance, ZeroHeuristic, target_id, plan);
    if (!status.ok()) {
      Log::Warnf("Couldn't complete GoBuySell due to FindPath: %s",
                 status.error_message());
      return;
    }
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

} // namespace

// Cost function returning the length of the connection.
micro::Measure ShortestDistance(const geography::Connection& conn) {
  return conn.length_u();
}

// Default heuristic that doesn't actually heurise.
micro::Measure ZeroHeuristic(const util::proto::ObjectId& cand_id,
                             const util::proto::ObjectId& target_id) {
  return 0;
}

// Adds to plan steps for traversing the connections between unit's current
// location and the provided target area.
util::Status FindPath(const units::Unit& unit,
                      const CostFunction& cost_function,
                      const Heuristic& heuristic,
                      const util::proto::ObjectId& target_id,
                      actions::proto::Plan* plan) {
  std::unordered_set<util::proto::ObjectId> open;
  struct Node {
    util::proto::ObjectId previous_id;
    uint64 conn_id;
    micro::Measure cost_u;
    micro::Measure heuristic_u;
  };
  std::unordered_map<util::proto::ObjectId, Node> visited;

  util::proto::ObjectId start_id = unit.location().a_area_id();
  if (start_id == target_id) {
    // Unit is either not going anywhere, or wants to turn around
    // having just set out from A.
    if (!unit.location().has_connection_id()) {
      DLOGF(Log::P_DEBUG, "FindPath start equals end %d, nothing to do",
            start_id.number());
      return util::OkStatus();
    }

    DLOGF(Log::P_DEBUG,
          "FindPath start equals end %d but in connection %d, turning around",
          start_id.number(), unit.location().connection_id());
    auto* step = plan->add_steps();
    step->set_action(actions::proto::AA_TURN_AROUND);
    step = plan->add_steps();
    step->set_action(actions::proto::AA_MOVE);
    step->set_connection_id(unit.location().connection_id());
    return util::OkStatus();
  }
  DLOGF(Log::P_DEBUG, "FindPath %d -> %d", start_id.number(),
        target_id.number());
  util::proto::ObjectId least_cost_id = start_id;
  micro::Measure least_cost_u = 0;
  open.insert(least_cost_id);
  visited.insert({least_cost_id, {util::objectid::kNullId, 0, least_cost_u}});
  // No closed set as we cannot guarantee the heuristic is consistent, and
  // besides it would interact badly with the possibility of multiple edges
  // between two nodes.

  // TODO: Include a give-up condition and handler.
  while (open.size() > 0) {
    const std::unordered_set<geography::Connection*>& conns =
        geography::Connection::ByEndpoint(least_cost_id);
    DLOGF(Log::P_DEBUG, "  Considering node %d with %d connections",
          least_cost_id.number(), conns.size());
    for (const auto* conn : conns) {
      util::proto::ObjectId other_side_id = conn->OtherSide(least_cost_id);
      micro::Measure real_cost_u = least_cost_u + cost_function(*conn);
      DLOGF(Log::P_DEBUG, "  Other side %d", other_side_id.number());
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
    for (auto& cand_id : open) {
      // TODO: Consider the tiebreak here.
      if (visited[cand_id].cost_u + visited[cand_id].heuristic_u >=
          least_cost_u) {
        continue;
      }
      least_cost_u = visited[cand_id].cost_u + visited[cand_id].heuristic_u;
      least_cost_id = cand_id;
    }
    if (util::objectid::Equal(least_cost_id, target_id)) {
      break;
    }
    least_cost_u = visited[least_cost_id].cost_u;
  }

  if (visited.find(target_id) == visited.end()) {
    DLOG(Log::P_DEBUG, "  Didn't find a path");
    // Didn't find a path.
    return util::NotFoundError(
        absl::Substitute("Couldn't find path from $0 to $1", start_id.number(),
                         target_id.number()));
  }

  std::vector<util::proto::ObjectId> path;
  util::proto::ObjectId current_id = target_id;
  while (!util::objectid::Equal(current_id, start_id)) {
    path.push_back(current_id);
    current_id = visited[current_id].previous_id;
  }

  // Unit has begun traversing a connection; it may have to turn around.
  if (unit.location().has_connection_id()) {
    if (unit.location().connection_id() != visited[path.back()].conn_id) {
      DLOGF(Log::P_DEBUG,
            "Need to turn around on connection %d before "
            "starting from area %d on connection %d",
            unit.location().connection_id(), start_id.number(),
            visited[path.back()].conn_id);
      auto* step = plan->add_steps();
      step->set_action(actions::proto::AA_TURN_AROUND);
      step = plan->add_steps();
      step->set_action(actions::proto::AA_MOVE);
      step->set_connection_id(unit.location().connection_id());
    }
  }

  while (!path.empty()) {
    const util::proto::ObjectId& id = path.back();
    DLOGF(Log::P_DEBUG, "  Path step: area %d by connection %d", id.number(),
          visited[id].conn_id);
    auto* step = plan->add_steps();
    step->set_action(actions::proto::AA_MOVE);
    step->set_connection_id(visited[id].conn_id);
    path.pop_back();
  }

  return util::OkStatus();
}

util::Status
ShuttleTrader::AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan* plan) {
  if (!strategy.has_shuttle_trade()) {
    return util::NotFoundError("No ShuttleTrader strategy");
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
  return util::OkStatus();
}

} // namespace impl
} // namespace ai
