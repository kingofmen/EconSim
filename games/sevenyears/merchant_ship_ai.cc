#include "games/sevenyears/merchant_ship_ai.h"

#include <algorithm>

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/executer.h"
#include "games/ai/planner.h"
#include "games/ai/impl/unit_ai_impl.h"
#include "games/ai/impl/ai_utils.h"
#include "games/geography/geography.h"
#include "games/industry/industry.h"
#include "games/market/goods_utils.h"
#include "games/sevenyears/constants.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/status/status.h"

namespace sevenyears {
namespace {

bool canDoEuropeanTrade(const geography::Area& area,
                        const sevenyears::proto::AreaState& state,
                        const util::proto::ObjectId faction_id) {
  if (faction_id == state.owner_id()) {
    return false;
  }

  // TODO: Don't rely purely on import capacity here.
  for (int i = 0; i < area.num_fields(); ++i) {
    const geography::proto::Field* field = area.field(i);
    if (field == nullptr) {
      continue;
    }
    if (market::GetAmount(field->resources(), constants::ImportCapacity()) >=
        micro::kOneInU) {
      return true;
    }
  }
  return false;
}

bool isMerchantShip(const units::Unit& unit) {
  for (const auto& tag : unit.Template().tags()) {
    if (tag == "merchant") {
      return true;
    }
  }
  return false;
}

bool isValidMission(const std::string& mission) {
  return mission == constants::EuropeanTrade() ||
         mission == constants::ColonialTrade() ||
         mission == constants::SupplyArmies();
}

void planTrade(const units::Unit& unit, const std::string& goods,
               actions::proto::Plan* plan) {
  DLOGF(Log::P_DEBUG, "Planning trade for unit %s in %s",
        unit.unit_id().DebugString(), goods);

  auto* step = plan->add_steps();
  step->set_key(constants::OffloadCargo());
  if (!goods.empty()) {
    step = plan->add_steps();
    step->set_key(constants::LoadShip());
    step->set_good(goods);
  }
}

// Remove cached information about the unit.
void clearUnitInfo(const units::Unit& unit, proto::Faction* faction) {}

// Returns the number of goods expected to be in the faction's warehouse
// in the area at the given timestamp.
// TODO: Account for unit cargo capacity.
micro::Measure netGoods(const util::proto::ObjectId& faction_id,
                        const std::string& goods, const proto::AreaState& state,
                        uint64 timestamp) {
  const proto::LocalFactionInfo* lfi = nullptr;
  for (int i = 0; i < state.factions_size(); ++i) {
    const auto& curr = state.factions(i);
    if (curr.faction_id() != faction_id) {
      continue;
    }
    lfi = &curr;
    break;
  }

  if (lfi == nullptr) {
    return 0;
  }

  // TODO: Also account for production.
  auto amount = market::GetAmount(lfi->warehouse(), goods);
  for (const auto& arr : lfi->arrivals()) {
    if (arr.timestamp() > timestamp) {
      continue;
    }
    amount += market::GetAmount(arr.cargo(), goods);
  }

  return amount;
}

// Returns the value of a hypothetical trade.
// TODO: Weighted sum?
// TODO: Use time, not distance!
micro::Measure tradePoints(micro::Measure goods, micro::Measure supplies,
                           micro::Measure distance) {
  micro::Measure value = goods + supplies;
  return micro::DivideU(value, distance);
}

}  // namespace

// Fills in the PlannedPath from the unit's current location to the provided
// area, if possible. Otherwise returns a non-OK status.
util::Status SevenYearsMerchant::createCandidatePath(
    const units::Unit& unit, const geography::Area& area,
    const util::proto::ObjectId& faction_id, PlannedPath* candidate) {
  const auto& state = game_->AreaState(area.area_id());
  if (!canDoEuropeanTrade(area, state, faction_id)) {
    return util::FailedPreconditionError(
        absl::Substitute("$0 cannot do European trade for $1",
                         util::objectid::DisplayString(area.area_id()),
                         util::objectid::DisplayString(faction_id)));
  }

  candidate->supply_source_id = area.area_id();
  std::vector<uint64> traverse;
  auto status =
      ai::impl::FindPath(unit.location(), ai::impl::ShortestDistance,
                         ai::impl::ZeroHeuristic, area.area_id(), &traverse);
  if (!status.ok()) {
    return status;
  }
  candidate->first_distance =
      ai::impl::CalculateDistance(traverse, ai::impl::ShortestDistance);
  int timeTaken = ai::utils::NumTurns(unit, traverse);
  candidate->supplies = netGoods(faction_id, constants::Supplies(), state,
                                 game_->timestamp() + timeTaken);
  return util::OkStatus();
}

void SevenYearsMerchant::checkForHomePort(
    const units::Unit& unit, const util::proto::ObjectId& area_id,
    const util::proto::ObjectId& faction_id, PlannedPath* candidate) {
  const auto& state = game_->AreaState(area_id);
  if (faction_id != state.owner_id()) {
    return;
  }
  std::vector<uint64> traverse;
  geography::proto::Location home_location;
  *home_location.mutable_a_area_id() = area_id;
  auto status = ai::impl::FindPath(home_location, ai::impl::ShortestDistance,
                                   ai::impl::ZeroHeuristic,
                                   candidate->supply_source_id, &traverse);
  if (!status.ok()) {
    // No safe path, but that's ok, just don't use this port.
    return;
  }

  // We can reach it; now consider whether this is a useful home port.
  micro::Measure dist =
      ai::impl::CalculateDistance(traverse, ai::impl::ShortestDistance);
  if (dist < candidate->home_distance) {
    candidate->home_distance = dist;
    candidate->dropoff_id = area_id;
    candidate->goodness = tradePoints(
        candidate->trade_goods, candidate->supplies,
        candidate->first_distance + candidate->outbound_distance +
            candidate->home_distance);
  }

  // Presumably the distance is the same both ways. So check if it is a
  // useful trade-supply port using the same distance.
  // TODO: If I ever implement prevailing winds this assumption may
  // change.
  std::vector<uint64> pathhome;
  status =
      ai::impl::FindPath(unit.location(), ai::impl::ShortestDistance,
                         ai::impl::ZeroHeuristic, area_id, &pathhome);
  if (!status.ok()) {
    // Problem with this specific port, not worth returning.
    DLOGF(Log::P_DEBUG, "Could not find path from %s to %s",
          unit.location().DebugString(), area_id.DebugString());
    return;
  }
  micro::Measure pickup_dist =
      ai::impl::CalculateDistance(pathhome, ai::impl::ShortestDistance);
  int timeTaken = ai::utils::NumTurns(unit, pathhome);
  micro::Measure availableTrade =
      netGoods(faction_id, constants::TradeGoods(), state,
               game_->timestamp() + timeTaken);
  // Supplies must account for the additional time taken to detour home.
  timeTaken += ai::utils::NumTurns(unit, traverse);
  micro::Measure availableSupplies = netGoods(
      faction_id, constants::Supplies(), state, game_->timestamp() + timeTaken);
  micro::Measure hypothetical =
      tradePoints(availableTrade, availableSupplies,
                  pickup_dist + dist + candidate->home_distance);
  if (hypothetical > candidate->goodness) {
    candidate->goodness = hypothetical;
    candidate->trade_goods = availableTrade;
    candidate->supplies = availableSupplies;
    candidate->first_distance = pickup_dist;
    candidate->outbound_distance = dist;
    candidate->trade_source_id = area_id;
  }
}

// The trade algorithm is: Sort the list of trading cities by priority,
// which is a combination of available supplies and import capacity (both
// adjusted by ships already on the way), closeness, and risk. Send the ship to
// the highest-priority city, optionally picking up trade goods on the way if
// necessary.
// TODO: Use a better heuristic in all find-path calculations.
// TODO: Actually account for risk.
util::Status SevenYearsMerchant::planEuropeanTrade(
    const units::Unit& unit, const actions::proto::SevenYearsMerchant& strategy,
    actions::proto::Plan* plan) {
  const auto& faction_id = unit.faction_id();
  auto& faction = factions_.at(faction_id);
  clearUnitInfo(unit, &faction);
  auto supplyCapacity = unit.Capacity(constants::Supplies());
  const auto& home_base_id = strategy.base_area_id();
  const auto& world = game_->World();

  // Create paths going straight to a trade port. Metric of goodness: Trade
  // goods delivered plus supplies brought home (both weighted) divided by time
  // taken. Later we will consider whether a home stop is an improvement. This
  // is caching the first half of the combinatorics.
  std::vector<PlannedPath> possible_paths;
  util::Status status;
  for (const auto& area : world.areas_) {
    PlannedPath candidate;
    status = createCandidatePath(unit, *area, faction_id, &candidate);
    if (!status.ok()) {
      // Indicates bad candidate, no need to propagate.
      continue;
    }
    possible_paths.push_back(candidate);
  }

  if (possible_paths.empty()) {
    return util::NotFoundError(absl::Substitute(
        "Could not find suitable trade port for $0; most recent problem: $1",
        util::objectid::DisplayString(unit.unit_id()),
        status.error_message().as_string()));
  }

  // Now add the possibility of a home-port stop; supply port is fixed in what
  // follows.
  // TODO: Use time, not distance. Distance is a distraction, time is the
  // important metric.
  for (auto& planned_path : possible_paths) {
    planned_path.home_distance = micro::kMaxU;
    for (const auto& area : world.areas_) {
      checkForHomePort(unit, area->area_id(), faction_id, &planned_path);
    }
  }
 
  int idx = 0;
  for (int i = 1; i < possible_paths.size(); ++i) {
    if (possible_paths[i].goodness < possible_paths[idx].goodness) {
      continue;
    }
    idx = i;
  }

  auto& path = possible_paths[idx];
  DLOGF(Log::P_DEBUG, "Unit %s picked path %d of %d: %s to %s to %s",
        unit.unit_id().DebugString(), idx, possible_paths.size(),
        util::objectid::IsNull(path.trade_source_id)
            ? "(none)"
            : util::objectid::Tag(path.trade_source_id),
        util::objectid::Tag(path.supply_source_id),
        util::objectid::Tag(path.dropoff_id));

  geography::proto::Location location = unit.location();
  if (!util::objectid::IsNull(path.trade_source_id)) {
    status =
        ai::impl::FindPath(unit, ai::impl::ShortestDistance,
                           ai::impl::ZeroHeuristic, path.trade_source_id, plan);
    DLOGF(Log::P_DEBUG, "Looked for path to trade source %s",
          path.trade_source_id.DebugString());
    if (!status.ok()) {
      return status;
    }
    *location.mutable_a_area_id() = path.trade_source_id;
    planTrade(unit, constants::TradeGoods(), plan);
  }

  std::vector<uint64> traverse;
  status = ai::impl::FindPath(location, ai::impl::ShortestDistance,
                              ai::impl::ZeroHeuristic, path.supply_source_id,
                              &traverse);
  if (!status.ok()) {
    return status;
  }
  ai::impl::PlanPath(location, traverse, plan);
  planTrade(unit, constants::Supplies(), plan);
  *location.mutable_a_area_id() = path.supply_source_id;

  traverse.clear();
  status =
      ai::impl::FindPath(location, ai::impl::ShortestDistance,
                         ai::impl::ZeroHeuristic, path.dropoff_id, &traverse);
  if (!status.ok()) {
    return status;
  }
  ai::impl::PlanPath(location, traverse, plan);
  planTrade(unit, "", plan);

  return util::OkStatus();
}

util::Status
SevenYearsMerchant::planColonialTrade(const units::Unit& unit,
                                      actions::proto::Plan* plan) const {
  return util::NotImplementedError("Colonial trade AI is not implemented.");
}

util::Status
SevenYearsMerchant::planSupplyArmies(const units::Unit& unit,
                                     actions::proto::Plan* plan) const {
  return util::NotImplementedError("Army supply AI is not implemented.");
}

util::Status SevenYearsMerchant::planReturnToBase(
    const units::Unit& unit, const actions::proto::SevenYearsMerchant& strategy,
    actions::proto::Plan* plan) const {
  auto status = ai::impl::FindPath(unit, ai::impl::ShortestDistance,
                                   ai::impl::ZeroHeuristic,
                                   strategy.base_area_id(), plan);
  if (!status.ok()) {
    return status;
  }
  auto* step = plan->add_steps();
  step->set_key(constants::OffloadCargo());
  return util::OkStatus();
}

util::Status
SevenYearsMerchant::AddStepsToPlan(const units::Unit& unit,
                                   const actions::proto::Strategy& strategy,
                                   actions::proto::Plan* plan) {
  if (!strategy.has_seven_years_merchant()) {
    return util::NotFoundError("No SevenYearsMerchant strategy");
  }
  if (plan->steps_size() > 0) {
    return util::OkStatus();
  }

  if (!isMerchantShip(unit)) {
    return util::InvalidArgumentError(absl::Substitute(
        "Unit $0 is not a merchant ship.", unit.unit_id().DebugString()));
  }

  if (factions_.find(unit.faction_id()) == factions_.end()) {
    factions_.emplace(std::make_pair(unit.faction_id(), proto::Faction{}));
  }

  const auto& merchant_strategy = strategy.seven_years_merchant();

  std::string mission = merchant_strategy.mission();
  if (mission.empty()) {
    mission = merchant_strategy.default_mission();
  }
  if (mission.empty()) {
    mission = constants::EuropeanTrade();
  }

  if (!isValidMission(mission)) {
    return util::InvalidArgumentError(
        absl::Substitute("Invalid mission '$0'", mission));
  }

  if (mission == constants::EuropeanTrade()) {
    return planEuropeanTrade(unit, merchant_strategy, plan);
  } else if (mission == constants::ColonialTrade()) {
    return planColonialTrade(unit, plan);
  } else if (mission == constants::SupplyArmies()) {
    return planSupplyArmies(unit, plan);
  }
  return util::NotImplementedError("This error should be unreachable.");
}

util::Status SevenYearsMerchant::Initialise() {
  actions::proto::Strategy strategy;
  strategy.mutable_seven_years_merchant()->mutable_base_area_id()->set_number(
      1);
  return ai::RegisterPlanner(strategy, this);
}

util::Status SevenYearsMerchant::ValidMission(
    const actions::proto::SevenYearsMerchant& sym) const {
  if (!sym.has_mission() && !sym.has_default_mission()) {
    return util::InvalidArgumentError("neither mission nor default mission");
  }
  if (sym.has_mission() && !isValidMission(sym.mission())) {
    return util::InvalidArgumentError(
        absl::Substitute("invalid mission $0", sym.mission()));
  }
  if (sym.has_default_mission() && !isValidMission(sym.default_mission())) {
    return util::InvalidArgumentError(
        absl::Substitute("invalid default mission $0", sym.default_mission()));
  }
  return util::OkStatus();
}

}  // namespace sevenyears
