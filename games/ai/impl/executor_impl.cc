#include "games/ai/impl/executor_impl.h"

#include "absl/strings/substitute.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/geography/connection.h"
#include "games/geography/proto/geography.pb.h"
#include "games/market/market.h"
#include "util/arithmetic/microunits.h"
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

  static geography::DefaultTraverser traverser;
  traverser.Traverse(*unit, location);

  return util::OkStatus();
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

} // namespace impl
} // namespace ai
