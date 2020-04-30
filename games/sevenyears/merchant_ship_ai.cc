#include "games/sevenyears/merchant_ship_ai.h"

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/executer.h"
#include "games/ai/planner.h"
#include "games/ai/impl/unit_ai_impl.h"
#include "games/industry/industry.h"
#include "games/market/goods_utils.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/status/status.h"

// TODO: Make 'inline' when we support C++17.
constexpr char kEuropeanTrade[] = "europe_trade";
constexpr char kColonialTrade[] = "colony_trade";
constexpr char kSupplyArmies[] = "supply_armies";
constexpr char kImportCapacity[] = "import_capacity";

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
    if (market::GetAmount(field->resources(), kImportCapacity) >=
        micro::kOneInU) {
      return true;
    }
  }
  return false;
}

util::Status europeanTrade(const actions::proto::Step& step, units::Unit* unit) {
  const auto& area_id = unit->location().a_area_id();
  Log::Debugf("Unit %s doing trade in %s",
              util::objectid::DisplayString(unit->unit_id()),
              util::objectid::DisplayString(area_id));
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
  return mission == kEuropeanTrade || mission == kColonialTrade ||
         mission == kSupplyArmies;
}

util::Status planTrade(const units::Unit& unit, const geography::Area& area,
                       const sevenyears::proto::AreaState& state,
                       actions::proto::Plan* plan) {
  auto* step = plan->add_steps();
  step->set_key(kEuropeanTrade);
  
  return util::OkStatus();
}

}  // namespace

util::Status
SevenYearsMerchant::EuropeanTrade(const units::Unit& unit,
                                  actions::proto::Plan* plan) const {
  const auto& world = game_->World();
  for (const auto& area : world.areas_) {
    const auto& area_id = area->area_id();
    if (area_id == unit.location().a_area_id()) {
      continue;
    }
    const auto& state = game_->AreaState(area_id);
    if (!canDoEuropeanTrade(unit, *area, state)) {
      continue;
    }

    // TODO: Distribute among available trade ports.
    // Better: If you can't do this one, try again elsewhere. Return fail
    // only if everything fails.
    // TODO: Better heuristic, and distance calculator that accounts for risk;
    // but that awaits warship implementation.
    auto status = ai::impl::FindPath(unit, ai::impl::ShortestDistance,
                                     ai::impl::ZeroHeuristic, area_id, plan);
    if (!status.ok()) {
      return status;
    }
    status = planTrade(unit, *area, state, plan);
    if (!status.ok()) {
      return status;
    }
  }
  return util::OkStatus();
}

util::Status
SevenYearsMerchant::ColonialTrade(const units::Unit& unit,
                                  actions::proto::Plan* plan) const {
  return util::NotImplementedError("Colonial trade AI is not implemented.");
}

util::Status
SevenYearsMerchant::SupplyArmies(const units::Unit& unit,
                                  actions::proto::Plan* plan) const {
  return util::NotImplementedError("Army supply AI is not implemented.");
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
  // TODO: Put in method, and add "load and unload" step.
  const auto& location = unit.location();
  if (location.a_area_id() != merchant_strategy.base_area_id() ||
      location.has_connection_id()) {
    return ai::impl::FindPath(unit, ai::impl::ShortestDistance,
                              ai::impl::ZeroHeuristic,
                              merchant_strategy.base_area_id(), plan);
  }

  // If in home base, choose where to go based on mission.
  std::string mission = merchant_strategy.mission();
  if (mission.empty()) {
    mission = merchant_strategy.default_mission();
  }
  if (mission.empty()) {
    mission = kEuropeanTrade;
  }

  if (!isValidMission(mission)) {
    return util::InvalidArgumentError(
        absl::Substitute("Invalid mission '$0'", mission));
  }

  if (mission == kEuropeanTrade) {
    return EuropeanTrade(unit, plan);
  } else if (mission == kColonialTrade) {
    return ColonialTrade(unit, plan);
  } else if (mission == kSupplyArmies) {
    return SupplyArmies(unit, plan);
  }
  return util::NotImplementedError("This error should be unreachable.");
}

util::Status SevenYearsMerchant::Initialise() {
  ai::RegisterExecutor(kEuropeanTrade, europeanTrade);
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
