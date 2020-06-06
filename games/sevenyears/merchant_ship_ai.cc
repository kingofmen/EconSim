#include "games/sevenyears/merchant_ship_ai.h"

#include <algorithm>

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/executer.h"
#include "games/ai/planner.h"
#include "games/ai/impl/unit_ai_impl.h"
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

bool canDoEuropeanTrade(const units::Unit& unit,
                        const geography::Area& area,
                        const sevenyears::proto::AreaState& state) {
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
  return mission == constants::EuropeanTrade() || mission == constants::ColonialTrade() ||
         mission == constants::SupplyArmies();
}

util::Status planTrade(const units::Unit& unit, const geography::Area& area,
                       const sevenyears::proto::AreaState& state,
                       actions::proto::Plan* plan) {
  auto* step = plan->add_steps();
  step->set_key(constants::OffloadCargo());
  step = plan->add_steps();
  step->set_key(constants::LoadShip());
  step->set_good(constants::Supplies());
  return util::OkStatus();
}

std::vector<geography::Area*> filterForTrade(const games::setup::World& world) {
  std::vector<geography::Area*> areas;
  for (const auto& area : world.areas_) {
    // TODO: Insert a filter here.
    areas.push_back(area.get());
  }

  // TODO: Should probably specify the random here.
  // TODO: But in any case it shouldn't be random, it should be sorted by
  // goodness and by how many ships have been sent recently; this is a stopgap.
  std::random_shuffle(areas.begin(), areas.end());
  return areas;
}

}  // namespace

util::Status
SevenYearsMerchant::planEuropeanTrade(const units::Unit& unit,
                                      actions::proto::Plan* plan) const {
  const auto& world = game_->World();
  const auto& home_base_id = unit.location().a_area_id();
  auto* step = plan->add_steps();
  step->set_key(constants::LoadShip());
  step->set_good(constants::TradeGoods());
  auto areas = filterForTrade(world);
  util::Status status;
  for (const auto* area : areas) {
    const auto& area_id = area->area_id();
    if (area_id == home_base_id) {
      continue;
    }
    const auto& state = game_->AreaState(area_id);
    if (!canDoEuropeanTrade(unit, *area, state)) {
      continue;
    }

    // TODO: Better heuristic, and distance calculator that accounts for risk;
    // but that awaits warship implementation.
    status = ai::impl::FindPath(unit, ai::impl::ShortestDistance,
                                ai::impl::ZeroHeuristic, area_id, plan);
    if (!status.ok()) {
      continue;
    }
    status = planTrade(unit, *area, state, plan);
    if (!status.ok()) {
      continue;
    }
    return util::OkStatus();
  }
  return util::NotFoundError(absl::Substitute(
      "Could not find suitable trade port for $0; most recent problem: $1",
      util::objectid::DisplayString(unit.unit_id()),
      status.error_message().as_string()));
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
                                   actions::proto::Plan* plan) const {
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

  const auto& merchant_strategy = strategy.seven_years_merchant();

  // If not in home base, go home.
  const auto& location = unit.location();
  if (location.a_area_id() != merchant_strategy.base_area_id() ||
      location.has_connection_id()) {
    return planReturnToBase(unit, merchant_strategy, plan);
  }

  // If in home base, choose where to go based on mission.
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
    return planEuropeanTrade(unit, plan);
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
