#include "games/sevenyears/merchant_ship_ai.h"

#include <algorithm>

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/executer.h"
#include "games/ai/impl/ai_utils.h"
#include "games/ai/impl/unit_ai_impl.h"
#include "games/ai/planner.h"
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

// If it takes longer than this, don't bother.
const int kMaxUsefulTime = 100;

util::Status canDoEuropeanTrade(const geography::Area& area,
                                const sevenyears::proto::AreaState& state,
                                const util::proto::ObjectId& faction_id) {
  if (faction_id == state.owner_id()) {
    return util::FailedPreconditionErrorf(
        "No European trade: %s owns %s",
        util::objectid::DisplayString(faction_id),
        util::objectid::DisplayString(area.area_id()));
  }

  // TODO: Don't rely purely on import capacity here.
  for (int i = 0; i < area.num_fields(); ++i) {
    const geography::proto::Field* field = area.field(i);
    if (field == nullptr) {
      continue;
    }
    if (market::GetAmount(field->resources(), constants::ImportCapacity()) >=
        micro::kOneInU) {
      return util::OkStatus();
    }
  }
  return util::FailedPreconditionErrorf(
      "No European trade in %s: No import capacity.",
      util::objectid::DisplayString(area.area_id()));
}

util::Status canDoArmySupply(const geography::Area& area,
                             const sevenyears::proto::AreaState& state,
                             const util::proto::ObjectId& faction_id) {
  return util::OkStatus();
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
  DLOGF(Log::P_DEBUG, "Planning trade for unit %s in \"%s\"",
        util::objectid::DisplayString(unit.unit_id()), goods);
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

// Returns the supplies consumed each turn by units of the given faction
// in the given area.
micro::Measure suppliesConsumed(const sevenyears::SevenYearsState& world,
                                const util::proto::ObjectId& faction_id,
                                const proto::AreaState& state) {
  const auto& area_id = state.area_id();
  micro::Measure consumed_u = 0;
  units::Filter filter;
  filter.location_id = area_id;
  filter.faction_id = faction_id;
  for (const auto* unit : world.ListUnits(filter)) {
    for (const auto& con : unit->Template().supplies()) {
      for (const auto& goods : con.goods()) {
        consumed_u +=
            market::GetAmount(goods.consumed(), constants::Supplies());
      }
    }
  }
  return consumed_u;
}

// Returns the value of a hypothetical trade.
// TODO: Weighted sum?
micro::Measure tradePoints(const PlannedPath& candidate) {
  micro::Measure value_u = candidate.carried_u + candidate.supplies;
  micro::Measure time = candidate.first_traverse_time +
                        candidate.pickup_to_target_time +
                        candidate.target_to_dropoff_time;
  return value_u / time;
}

micro::Measure supplyPoints(const PlannedPath& candidate) {
  const auto* unit = units::ById(candidate.unit_id);
  if (unit == nullptr) {
    return 0;
  }
  if (candidate.supplies == 0) {
    return 0;
  }
  if (candidate.carried_u == 0) {
    return 0;
  }
  auto delivered_u = micro::MultiplyU(candidate.carried_u, candidate.supplies);
  micro::Measure time_u = candidate.first_traverse_time +
                          candidate.pickup_to_target_time +
                          candidate.target_to_dropoff_time;
  time_u *= micro::kOneInU;
  return micro::DivideU(delivered_u, time_u);
}

} // namespace

// Fills in the PlannedPath from the unit's current location to the provided
// area, if possible. Otherwise returns a non-OK status.
util::Status SevenYearsMerchant::createCandidatePath(
    const units::Unit& unit, const geography::Area& area,
    const util::proto::ObjectId& faction_id, MissionCheck pred,
    PlannedPath* candidate) {
  const auto& state = game_->AreaState(area.area_id());
  auto status = pred(area, state, faction_id);
  if (!status.ok()) {
    return status;
  }

  std::vector<geography::Connection::IdType> traverse;
  status =
      ai::impl::FindPath(unit.location(), ai::impl::ShortestDistance,
                         ai::impl::ZeroHeuristic, area.area_id(), &traverse);
  if (!status.ok()) {
    return status;
  }
  candidate->unit_id = unit.unit_id();
  candidate->target_port_id = area.area_id();
  candidate->first_traverse_time = ai::utils::NumTurns(unit, traverse);
  candidate->pickup_to_target_time = 0;
  candidate->target_to_dropoff_time = 0;
  candidate->goodness = 0;
  candidate->carried_u = 0;
  return util::OkStatus();
}

void SevenYearsMerchant::checkForDropoff(
    const units::Unit& unit, const util::proto::ObjectId& area_id,
    const util::proto::ObjectId& faction_id, GoodnessMetric metric,
    PlannedPath* candidate) {
  const auto& state = game_->AreaState(area_id);
  if (faction_id != state.owner_id()) {
    return;
  }
  std::vector<geography::Connection::IdType> traverse;
  geography::proto::Location home_location;
  *home_location.mutable_a_area_id() = area_id;
  auto status = ai::impl::FindPath(home_location, ai::impl::ShortestDistance,
                                   ai::impl::ZeroHeuristic,
                                   candidate->target_port_id, &traverse);
  if (!status.ok()) {
    // No safe path, but that's ok, just don't use this port as the dropoff.
    return;
  }

  // We can reach it; now consider whether this is a useful dropoff, i.e.
  // final, port.
  micro::Measure dropoff_time = ai::utils::NumTurns(unit, traverse);
  if (dropoff_time < candidate->target_to_dropoff_time) {
    candidate->target_to_dropoff_time = dropoff_time;
    candidate->dropoff_id = area_id;
    candidate->goodness = metric(*candidate);
  }
}

// Calculates the goodness of a pickup stop in the target area,
// and changes candidate to use this pickup if it's better than
// the current one.
void SevenYearsMerchant::checkForPickup(const units::Unit& unit,
                                        const util::proto::ObjectId& area_id,
                                        const util::proto::ObjectId& faction_id,
                                        const std::string& pickupGoods,
                                        const std::string& exchangeGoods,
                                        GoodnessMetric metric,
                                        PlannedPath* candidate) {
  if (area_id == candidate->target_port_id) {
    return;
  }
  const auto& state = game_->AreaState(area_id);
  if (faction_id != state.owner_id()) {
    return;
  }
  std::vector<geography::Connection::IdType> carry_path;
  geography::proto::Location home_location;
  // Check if we can get from the area to the target.
  *home_location.mutable_a_area_id() = area_id;
  auto status = ai::impl::FindPath(home_location, ai::impl::ShortestDistance,
                                   ai::impl::ZeroHeuristic,
                                   candidate->target_port_id, &carry_path);
  if (!status.ok()) {
    // No safe path, but that's ok, just don't use this port for pickup.
    return;
  }

  std::vector<geography::Connection::IdType> pickup_path;
  status = ai::impl::FindPath(unit.location(), ai::impl::ShortestDistance,
                              ai::impl::ZeroHeuristic, area_id, &pickup_path);
  if (!status.ok()) {
    // Problem with this specific port, not worth returning.
    DLOGF(Log::P_DEBUG, "Could not find path from %s to %s for pickup",
          unit.location().DebugString(),
          util::objectid::DisplayString(area_id));
    return;
  }
  int pickup_time = ai::utils::NumTurns(unit, pickup_path);
  micro::Measure availablePickup = netGoods(faction_id, pickupGoods, state,
                                            game_->timestamp() + pickup_time);
  // Supplies must account for the additional time taken to detour to pickup.
  int carry_time = ai::utils::NumTurns(unit, carry_path);

  PlannedPath hypothetical;
  hypothetical.unit_id = candidate->unit_id;
  hypothetical.supplies = candidate->supplies;
  micro::Measure availableExchange = 0;
  if (!exchangeGoods.empty()) {
    availableExchange = netGoods(faction_id, exchangeGoods, state,
                                 game_->timestamp() + pickup_time + carry_time);
    hypothetical.supplies = availableExchange;
  }

  hypothetical.carried_u = availablePickup;
  hypothetical.first_traverse_time = pickup_time;
  hypothetical.pickup_to_target_time = carry_time;
  hypothetical.target_to_dropoff_time = candidate->target_to_dropoff_time;
  auto hypoGoodness = metric(hypothetical);
  if (hypoGoodness > candidate->goodness) {
    candidate->goodness = hypoGoodness;
    candidate->carried_u = availablePickup;
    candidate->supplies = availableExchange;
    candidate->first_traverse_time = pickup_time;
    candidate->pickup_to_target_time = carry_time;
    candidate->pickup_id = area_id;
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
  // This line is necessary but I don't understand why. TODO: Remove it.
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
    status = createCandidatePath(unit, *area, faction_id, canDoEuropeanTrade,
                                 &candidate);
    if (!status.ok()) {
      // Indicates bad candidate, no need to propagate.
      continue;
    }
    const auto& state = game_->AreaState(area->area_id());
    candidate.supplies =
        netGoods(faction_id, constants::Supplies(), state,
                 game_->timestamp() + candidate.first_traverse_time);
    if (candidate.supplies > supplyCapacity) {
      candidate.supplies = supplyCapacity;
    }
    possible_paths.push_back(candidate);
  }

  if (possible_paths.empty()) {
    return util::NotFoundError(absl::Substitute(
        "Could not find suitable trade port for $0; most recent problem: $1",
        util::objectid::DisplayString(unit.unit_id()),
        status.message()));
  }

  // Check if we want this to be the dropoff port.
  for (auto& planned_path : possible_paths) {
    planned_path.target_to_dropoff_time = kMaxUsefulTime;
    for (const auto& area : world.areas_) {
      checkForDropoff(unit, area->area_id(), faction_id, tradePoints,
                      &planned_path);
    }
  }

  // Check whether this can be the pickup port.
  for (auto& planned_path : possible_paths) {
    for (const auto& area : world.areas_) {
      checkForPickup(unit, area->area_id(), faction_id, constants::TradeGoods(),
                     constants::Supplies(), tradePoints, &planned_path);
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
  DLOGF(Log::P_DEBUG, "Trade unit %s picked path %d of %d: %s to %s to %s",
        util::objectid::DisplayString(unit.unit_id()), idx,
        possible_paths.size(),
        util::objectid::IsNull(path.pickup_id)
            ? "(none)"
            : util::objectid::Tag(path.pickup_id),
        util::objectid::Tag(path.target_port_id),
        util::objectid::Tag(path.dropoff_id));
  geography::proto::Location location = unit.location();
  if (!util::objectid::IsNull(path.pickup_id)) {
    status = ai::impl::FindPath(unit, ai::impl::ShortestDistance,
                                ai::impl::ZeroHeuristic, path.pickup_id, plan);
    DLOGF(Log::P_DEBUG, "Looked for path to trade source %s",
          path.pickup_id.DebugString());
    if (!status.ok()) {
      return status;
    }
    *location.mutable_a_area_id() = path.pickup_id;
    planTrade(unit, constants::TradeGoods(), plan);
  }

  std::vector<geography::Connection::IdType> traverse;
  status = ai::impl::FindPath(location, ai::impl::ShortestDistance,
                              ai::impl::ZeroHeuristic, path.target_port_id,
                              &traverse);
  if (!status.ok()) {
    return status;
  }
  ai::impl::PlanPath(location, traverse, plan);
  planTrade(unit, constants::Supplies(), plan);
  *location.mutable_a_area_id() = path.target_port_id;

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

// The supply algorithm is: Sort the list of supply-requiring army bases
// by priority, which is a combination of supplies used, supplies already
// present (including on the way), closeness, and risk. Send the ship to
// the highest priority, adding a detour to pick up the supplies if needed.
util::Status SevenYearsMerchant::planSupplyArmies(const units::Unit& unit,
                                                  actions::proto::Plan* plan) {
  const auto& faction_id = unit.faction_id();
  auto& faction = factions_.at(faction_id);
  clearUnitInfo(unit, &faction);
  auto supplyCapacity = unit.Capacity(constants::Supplies());
  const auto& world = game_->World();

  // Create paths going straight to an army base.
  std::vector<PlannedPath> possible_paths;
  util::Status status;
  for (const auto& area : world.areas_) {
    PlannedPath candidate;
    status = createCandidatePath(unit, *area, faction_id, canDoArmySupply,
                                 &candidate);
    if (!status.ok()) {
      // Indicates bad candidate, no need to propagate.
      continue;
    }
    const auto& state = game_->AreaState(area->area_id());
    candidate.supplies = suppliesConsumed(*game_, faction_id, state);
    candidate.carried_u =
        market::GetAmount(unit.resources(), constants::Supplies());
    candidate.goodness = supplyPoints(candidate);
    possible_paths.push_back(candidate);
  }
  if (possible_paths.empty()) {
    return util::NotFoundErrorf(
        "Could not find suitable supply target for %s; most recent problem: %s",
        util::objectid::DisplayString(unit.unit_id()),
        status.message());
  }

  for (auto& planned_path : possible_paths) {
    // TODO: Filter the area list so this isn't quite O(n^2).
    for (const auto& area : world.areas_) {
      checkForPickup(unit, area->area_id(), faction_id, constants::Supplies(),
                     "", supplyPoints, &planned_path);
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
  DLOGF(Log::P_DEBUG,
        "Supply unit %s picked path %d of %d with goodness %d: %s to %s",
        util::objectid::DisplayString(unit.unit_id()), idx,
        possible_paths.size(), path.goodness,
        util::objectid::IsNull(path.pickup_id)
            ? "(none)"
            : util::objectid::Tag(path.pickup_id),
        util::objectid::Tag(path.target_port_id));

  geography::proto::Location location = unit.location();
  if (!util::objectid::IsNull(path.pickup_id)) {
    status = ai::impl::FindPath(unit, ai::impl::ShortestDistance,
                                ai::impl::ZeroHeuristic, path.pickup_id, plan);
    DLOGF(Log::P_DEBUG, "Looked for path to supply source %s",
          util::objectid::DisplayString(path.pickup_id));
    if (!status.ok()) {
      return status;
    }
    *location.mutable_a_area_id() = path.pickup_id;
    planTrade(unit, constants::Supplies(), plan);
  }

  std::vector<geography::Connection::IdType> pathToTarget;
  status = ai::impl::FindPath(location, ai::impl::ShortestDistance,
                              ai::impl::ZeroHeuristic, path.target_port_id,
                              &pathToTarget);
  if (!status.ok()) {
    return status;
  }
  ai::impl::PlanPath(location, pathToTarget, plan);
  planTrade(unit, "", plan);

  return util::OkStatus();
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
    return util::InvalidArgumentErrorf(
        "Unit %s is not a merchant ship.",
        util::objectid::DisplayString(unit.unit_id()));
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
  return util::NotImplementedErrorf("This error should be unreachable: %s",
                                    plan->DebugString());
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

} // namespace sevenyears
