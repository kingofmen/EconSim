#include "executor_impl.h"

#include "actions/proto/strategy.pb.h"
#include "geography/connection.h"
#include "geography/proto/geography.pb.h"
#include "market/market.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.h"

namespace ai {
namespace impl {

bool MoveUnit(const actions::proto::Step& step, units::Unit* unit) {
  if (!step.has_connection_id()) {
    return false;
  }

  geography::proto::Location* location = unit->mutable_location();
  if (location->has_connection_id()) {
    if (location->connection_id() != step.connection_id()) {
      return false;
    }
  } else {
    if (!location->has_a_area_id()) {
      return false;
    }
    const auto* connection = geography::Connection::ById(step.connection_id());
    if (!connection) {
      DLOGF(Log::P_DEBUG, "  MoveUnit could not find connection ID %d",
            step.connection_id());
      return false;
    }

    if (util::objectid::IsNull(connection->OtherSide(location->a_area_id()))) {
      return false;
    }

    // Step into the connection before starting traverse.
    location->set_connection_id(step.connection_id());
  }

  static geography::DefaultTraverser traverser;
  traverser.Traverse(*unit, location);

  return true;
}

bool BuyOrSell(const actions::proto::Step& step, units::Unit* unit) {
  if (!step.has_good()) {
    return false;
  }
  if (step.action() != actions::proto::AA_BUY &&
      step.action() != actions::proto::AA_SELL) {
    return false;
  }

  const geography::proto::Location& location = unit->location();
  if (location.has_progress_u() && location.progress_u() > 0) {
    // Cannot buy while in a Connection.
    return false;
  }

  geography::Area* area = geography::Area::GetById(location.a_area_id());
  if (area == NULL) {
    // Something is really weird.
    // TODO: Fail more obviously here, this should never happen.
    return false;
  }

  market::Market* market = area->mutable_market();
  if (market == NULL) {
    // This should probably never happen either.
    return false;
  }

  if (step.action() == actions::proto::AA_BUY) {
    market->TryToBuy(step.good(), unit->Capacity(step.good()), unit->mutable_resources());
  } else {
    market->TryToSell(step.good(),
                      market::GetAmount(unit->resources(), step.good()),
                      unit->mutable_resources());
  }

  return true;
}

bool SwitchState(const actions::proto::Step& step, units::Unit* unit) {
  actions::proto::Strategy* strat = unit->mutable_strategy();
  switch(strat->strategy_case()) {
    case actions::proto::Strategy::kShuttleTrade:
      {
        actions::proto::ShuttleTrade* info = strat->mutable_shuttle_trade();
        if (info->state() == actions::proto::ShuttleTrade::STS_BUY_A) {
          info->set_state(actions::proto::ShuttleTrade::STS_BUY_Z);
        } else {
          info->set_state(actions::proto::ShuttleTrade::STS_BUY_A);
        }
        return true;
      }
    default:
      break;
  }
  return false;
}

} // namespace impl
} // namespace ai
