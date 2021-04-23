#include "games/ai/public/cost.h"

#include "games/geography/connection.h"
#include "util/logging/logging.h"

namespace ai {

ActionCost ZeroCost() {
  return ActionCost(0, micro::kOneInU);
}

ActionCost OneCost() {
  return ActionCost(micro::kOneInU, micro::kOneInU);
}

ActionCost InfiniteCost() {
  return ActionCost(micro::kuMaxU, 0);
}

ActionCost ZeroCost(const actions::proto::Step&, const units::Unit&) {
  return ZeroCost();
}

ActionCost OneCost(const actions::proto::Step&, const units::Unit&) {
  return OneCost();
}

ActionCost InfiniteCost(const actions::proto::Step&, const units::Unit&) {
  return InfiniteCost();
}

ActionCost DefaultMoveCost(const actions::proto::Step& step, const units::Unit& unit) {
  if (step.trigger_case() != actions::proto::Step::kAction) {
    return InfiniteCost();
  }
  if (!step.has_connection_id()) {
    return InfiniteCost();
  }
  const auto& points_u = unit.action_points_u();
  if (points_u < 1) {
    return OneCost();
  }

  const geography::proto::Location& location = unit.location();
  const geography::Connection* connection = nullptr;
  if (location.has_connection_id()) {
    connection = geography::Connection::ById(location.connection_id());
  } else {
    connection = geography::Connection::ById(step.connection_id());
  }
  if (!connection) {
    DLOGF(Log::P_DEBUG, "  DefaultMoveCost could not find connection ID %d",
          location.has_connection_id() ? location.connection_id()
                                       : step.connection_id());
    return InfiniteCost();
  }

  micro::uMeasure progress_u = location.progress_u();
  micro::uMeasure distance_u = connection->length_u() - progress_u;
  if (distance_u < 1) {
    return ZeroCost();
  }
  micro::uMeasure speed_u = unit.speed_u(connection->type());
  if (distance_u >= speed_u) {
    // We won't finish this in one step.
    return ActionCost(std::min(micro::kOneInU, points_u),
                      micro::DivideU(speed_u, distance_u));
  }

  return ActionCost(micro::DivideU(distance_u, speed_u), micro::kOneInU);
}

ActionCost PartialCost(const actions::proto::Step& /*step*/,
                       const units::Unit& unit) {
  return PartialCost(unit);
}

ActionCost PartialCost(const units::Unit& unit) {
  const auto points_u = unit.action_points_u();
  if (points_u < 1) {
    return OneCost();
  }
  if (points_u >= micro::kOneInU) {
    return OneCost();
  }
  // Partial action.
  return ActionCost(points_u, points_u);
}


} // namespace ai
