#include "games/ai/impl/executor_impl.h"

#include "absl/strings/substitute.h"
#include "games/ai/impl/ai_utils.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/geography/connection.h"
#include "games/geography/proto/geography.pb.h"
#include "games/market/goods_utils.h"
#include "games/market/market.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.h"

namespace ai {
namespace impl {

namespace {

util::Status validateMove(const actions::proto::Step& step, const geography::proto::Location& location) {
  if (location.has_connection_id() &&
      location.connection_id() != step.connection_id()) {
    return util::NotFoundErrorf(
        "Unit in connection %s trying to move in connection %s",
        util::objectid::DisplayString(location.connection_id()),
        util::objectid::DisplayString(step.connection_id()));
  }

  if (!location.has_a_area_id()) {
    return util::NotFoundError("Unit trying to move without A location");
  }
  const auto* connection = geography::Connection::ById(step.connection_id());
  if (!connection) {
    DLOGF(Log::P_DEBUG, "  validateMove could not find %s",
          util::objectid::DisplayString(step.connection_id()));
    return util::NotFoundErrorf(
        "Could not find %s",
        util::objectid::DisplayString(step.connection_id()));
  }

  if (util::objectid::IsNull(connection->OtherSide(location.a_area_id()))) {
    return util::NotFoundErrorf(
        "Could not find area on other side of %s through %s",
        util::objectid::DisplayString(location.a_area_id()),
        util::objectid::DisplayString(step.connection_id()));
  }

  return util::OkStatus();
}

} // namespace

util::Status MoveUnit(const ActionCost& cost, const actions::proto::Step& step,
                      units::Unit* unit) {
  const auto& unit_id = unit->unit_id();
  if (!step.has_connection_id()) {
    return util::InvalidArgumentErrorf("%s cannot move without connection ID",
                                       util::objectid::DisplayString(unit_id));
  }

  geography::proto::Location* location = unit->mutable_location();
  auto status = validateMove(step, *location);
  if (!status.ok()) {
    return status;
  }

  // Ensure there is a connection to traverse. Validation done above.
  const auto& connection_id = step.connection_id();
  *location->mutable_connection_id() = connection_id;
  // Connection known to exist from validation.
  const auto* connection = geography::Connection::ById(connection_id);
  uint64 progress_u = location->progress_u();
  uint64 length_u = connection->length_u() - progress_u;
  uint64 distance_u =
      ai::utils::GetProgress(cost.fraction_u, *unit, *connection);
  const auto& a_area_id = location->a_area_id();
  const auto& z_area_id = connection->OtherSide(a_area_id);
  DLOGF(Log::P_DEBUG, "Unit %s progress %s moving from %s to %s",
        util::objectid::DisplayString(unit_id),
        micro::DisplayString(distance_u, 2),
        util::objectid::DisplayString(a_area_id),
        util::objectid::DisplayString(z_area_id));

  geography::Connection::Movement movement(connection_id, unit_id, a_area_id,
                                           progress_u, distance_u);
  connection->Listen(movement);
  if (distance_u >= length_u) {
    location->clear_progress_u();
    *location->mutable_a_area_id() = z_area_id;
    location->clear_connection_id();
    return util::OkStatus();
  } else {
    location->set_progress_u(progress_u + distance_u);
    return util::NotComplete();
  }
}

util::Status BuyOrSell(const ActionCost& cost, const actions::proto::Step& step,
                       units::Unit* unit) {
  if (!step.has_good()) {
    return util::InvalidArgumentError("Cannot do BuyOrSell without good");
  }

  const geography::proto::Location& location = unit->location();
  if (location.has_progress_u() && location.progress_u() > 0) {
    // Cannot buy while in a Connection.
    return util::InvalidArgumentErrorf(
        "BuyOrSell while still traversing connection %s",
        util::objectid::DisplayString(location.connection_id()));
  }

  geography::Area* area = geography::Area::GetById(location.a_area_id());
  if (area == NULL) {
    return util::NotFoundErrorf(
        "BuyOrSell in nonexistent %s",
        util::objectid::DisplayString(location.a_area_id()));
  }

  market::Market* market = area->mutable_market();
  if (market == NULL) {
    return util::NotFoundErrorf(
        "BuyOrSell in %s which has no market",
        util::objectid::DisplayString(location.a_area_id()));
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

util::Status SwitchState(const ActionCost& cost,
                         const actions::proto::Step& step, units::Unit* unit) {
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

util::Status TurnAround(const ActionCost& cost,
                        const actions::proto::Step& step, units::Unit* unit) {
  auto* location = unit->mutable_location();
  if (!location->has_connection_id()) {
    return util::FailedPreconditionErrorf(
        "%s tried to turn around but is not in a connection",
        util::objectid::DisplayString(unit->unit_id()));
  }

  const auto* connection =
      geography::Connection::ById(location->connection_id());
  if (connection == nullptr) {
    return util::NotFoundErrorf(
        "%s tried to turn around but is in nonexistent %s",
        util::objectid::DisplayString(unit->unit_id()),
        util::objectid::DisplayString(location->connection_id()));
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
