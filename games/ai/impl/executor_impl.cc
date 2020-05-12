#include "games/ai/impl/executor_impl.h"

#include "absl/strings/substitute.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/geography/connection.h"
#include "games/geography/proto/geography.pb.h"
#include "games/market/goods_utils.h"
#include "games/market/market.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.h"

namespace ai {
namespace impl {

util::Status MoveUnit(const actions::proto::Step& step, units::Unit* unit) {
  if (!step.has_connection_id()) {
    return util::InvalidArgumentError("Cannot move without connection ID");
  }

  geography::proto::Location* location = unit->mutable_location();
  if (location->has_connection_id()) {
    if (location->connection_id() != step.connection_id()) {
      return util::NotFoundError(absl::Substitute(
          "Unit in connection $0 trying to move in connection $1",
          location->connection_id(), step.connection_id()));
    }
  } else {
    if (!location->has_a_area_id()) {
      return util::NotFoundError("Unit trying to move without A location");
    }
    const auto* connection = geography::Connection::ById(step.connection_id());
    if (!connection) {
      DLOGF(Log::P_DEBUG, "  MoveUnit could not find connection ID %d",
            step.connection_id());
      return util::NotFoundError(absl::Substitute(
          "Could not find connection ID $0", step.connection_id()));
    }

    if (util::objectid::IsNull(connection->OtherSide(location->a_area_id()))) {
      return util::NotFoundError(absl::Substitute(
          "Could not find area on other side of area $0 through connection $1",
          location->a_area_id().number(), step.connection_id()));
    }

    // Step into the connection before starting traverse.
    location->set_connection_id(step.connection_id());
  }

  auto* connection = geography::Connection::ById(step.connection_id());
  if (connection == nullptr) {
    return util::NotFoundError(
        absl::Substitute("Could not find connection $0", step.connection_id()));
  }

  uint64 progress_u = location->progress_u();
  uint64 distance_u = unit->speed_u(connection->type());
  uint64 length_u = connection->length_u() - progress_u;
  if (distance_u > length_u) {
    distance_u = length_u;
  }

  // TODO: Add a detections vector, and handle them.
  connection->Listen(*unit, distance_u, nullptr);
  if (distance_u >= length_u) {
    location->clear_progress_u();
    *location->mutable_a_area_id() =
        connection->OtherSide(location->a_area_id());
    location->clear_connection_id();
    return util::OkStatus();
  } else {
    location->set_progress_u(progress_u + distance_u);
    return util::NotComplete();
  }
}

util::Status BuyOrSell(const actions::proto::Step& step, units::Unit* unit) {
  if (!step.has_good()) {
    return util::InvalidArgumentError("Cannot do BuyOrSell without good");
  }

  const geography::proto::Location& location = unit->location();
  if (location.has_progress_u() && location.progress_u() > 0) {
    // Cannot buy while in a Connection.
    return util::InvalidArgumentError(
        absl::Substitute("BuyOrSell while still traversing connection $0",
                         location.connection_id()));
  }

  geography::Area* area = geography::Area::GetById(location.a_area_id());
  if (area == NULL) {
    return util::NotFoundError(absl::Substitute(
        "BuyOrSell in nonexistent area $0", location.a_area_id().number()));
  }

  market::Market* market = area->mutable_market();
  if (market == NULL) {
    return util::NotFoundError(
        absl::Substitute("BuyOrSell in area $0 which has no market",
                         location.a_area_id().number()));
  }

  if (step.action() == actions::proto::AA_BUY) {
    market->TryToBuy(step.good(), unit->Capacity(step.good()),
                     unit->mutable_resources());
  } else {
    market->TryToSell(step.good(),
                      market::GetAmount(unit->resources(), step.good()),
                      unit->mutable_resources());
  }

  return util::OkStatus();
}

util::Status SwitchState(const actions::proto::Step& step, units::Unit* unit) {
  actions::proto::Strategy* strat = unit->mutable_strategy();
  switch (strat->strategy_case()) {
  case actions::proto::Strategy::kShuttleTrade: {
    actions::proto::ShuttleTrade* info = strat->mutable_shuttle_trade();
    if (info->state() == actions::proto::ShuttleTrade::STS_BUY_A) {
      info->set_state(actions::proto::ShuttleTrade::STS_BUY_Z);
    } else {
      info->set_state(actions::proto::ShuttleTrade::STS_BUY_A);
    }
    return util::OkStatus();
  }
  default:
    break;
  }
  return util::NotImplementedError(
      absl::Substitute("SwitchState for strategy case $0 not implemented"));
}

util::Status TurnAround(const actions::proto::Step& step, units::Unit* unit) {
  auto* location = unit->mutable_location();
  if (!location->has_connection_id()) {
    return util::FailedPreconditionError(absl::Substitute(
        "Unit ($0, $0) tried to turn around but is not in a connection",
        unit->unit_id().kind(), unit->unit_id().number()));
  }

  const auto* connection =
      geography::Connection::ById(location->connection_id());
  if (connection == nullptr) {
    return util::NotFoundError(
        absl::Substitute("Unit ($0, $0) tried to turn around but is in "
                         "nonexistent connection %d",
                         unit->unit_id().kind(), unit->unit_id().number(),
                         location->connection_id()));
  }

  auto progress_u = connection->length_u();
  progress_u -= location->progress_u();
  if (progress_u < 0) {
    progress_u = 0;
  }
  const auto& other_side_id = connection->OtherSide(location->a_area_id());
  *(location->mutable_a_area_id()) = other_side_id;
  location->set_progress_u(progress_u);
  return util::OkStatus();
}


} // namespace impl
} // namespace ai
