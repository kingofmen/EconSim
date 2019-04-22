#include "executor_impl.h"


#include "geography/connection.h"
#include "geography/proto/geography.pb.h"
#include "market/market.h"
#include "util/arithmetic/microunits.h"

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
    if (!location->has_source_area_id()) {
      return false;
    }
    if (geography::Connection::ById(step.connection_id())
            ->OtherSide(location->source_area_id()) == 0) {
      return false;
    }
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

  geography::Area* area = geography::Area::GetById(location.source_area_id());
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

  // TODO: Cargo capacity.
  if (step.action() == actions::proto::AA_BUY) {
    market->TryToBuy(step.good(), micro::kOneInU, unit->mutable_resources());
  } else {
    market->TryToSell(step.good(),
                      market::GetAmount(unit->resources(), step.good()),
                      unit->mutable_resources());
  }
  return true;
}

} // namespace impl
} // namespace ai
