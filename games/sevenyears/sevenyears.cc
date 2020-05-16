#include "games/sevenyears/sevenyears.h"

#include <unordered_set>

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/actions/strategy.h"
#include "games/ai/executer.h"
#include "games/ai/planner.h"
#include "games/industry/industry.h"
#include "games/market/goods_utils.h"
#include "games/setup/proto/setup.pb.h"
#include "games/setup/setup.h"
#include "games/setup/validation/validation.h"
#include "games/sevenyears/constants.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/merchant_ship_ai.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/status/status.h"

namespace sevenyears {

namespace {

constexpr int kMaxIncompleteWait = 5;

util::Status validateSetup(const games::setup::proto::ScenarioFiles& setup) {
  if (!setup.has_name()) {
    return util::InvalidArgumentError(
        absl::Substitute("Setup file has no name"));
  }
  if (!setup.has_description()) {
    return util::InvalidArgumentError(
        absl::Substitute("$0 has no description", setup.name()));
  }
  if (!setup.has_world_file()) {
    return util::InvalidArgumentError(
        absl::Substitute("$0 has no world_file", setup.name()));
  }
  return util::OkStatus();
}

util::Status validateWorldState(
    const std::unordered_map<std::string, industry::Production>& chains,
    proto::WorldState* state) {
  for (int i = 0; i < state->area_states_size(); ++i) {
    auto* area_state = state->mutable_area_states(i);
    auto status = util::objectid::Canonicalise(area_state->mutable_area_id());
    if (!status.ok()) {
      return status;
    }
    const auto& area_id = area_state->area_id();
    auto* area = geography::Area::GetById(area_id);
    if (area == nullptr) {
      return util::NotFoundError(
          absl::Substitute("Could not find area $0", area_id.DebugString()));
    }

    int numFields = area->num_fields();
    int numProd = area_state->production_size();
    if (numFields < numProd) {
      Log::Warnf(
          "In area %d: %d fields, %d production strings, ignoring surplus",
          area_id.number(), numFields, numProd);
      area_state->mutable_production()->DeleteSubrange(numFields,
                                                       numProd - numFields);
    }

    if (!market::AllGoodsExist(area_state->warehouse())) {
      for (const auto& q : area_state->warehouse().quantities()) {
        if (market::Exists(q.first)) {
          continue;
        }
        Log::Errorf("In area %d: Good %s does not exist.", area_id.number(), q.first);
      }
      return util::NotFoundError(absl::Substitute(
          "Not all goods in area $0 warehouse exist.", area_id.number()));
    }

    numProd = area_state->production_size();
    for (int i = 0; i < numProd; ++i) {
      const auto& prod = area_state->production(i);
      if (chains.find(prod) == chains.end()) {
        return util::NotFoundError(absl::Substitute(
            "Invalid production type $0 in area $1", prod, area_id.number()));
      }
    }
  }

  return util::OkStatus();
}

}  // namespace


util::Status SevenYears::InitialiseAI() {
  ai::RegisterExecutor(
      constants::EuropeanTrade(),
      [this](const actions::proto::Step& step, units::Unit* unit) {
        return this->doEuropeanTrade(step, unit);
      });
  ai::RegisterExecutor(
      constants::LoadShip(),
      [this](const actions::proto::Step& step, units::Unit* unit) {
        return this->loadShip(step, unit);
      });

  merchant_ai_.reset(new SevenYearsMerchant(this));
  ai::RegisterCost(actions::proto::AA_MOVE, ai::DefaultMoveCost);
  cost_calculator_.reset(new ActionCostCalculator(this));
  ai::RegisterDefaultCost(*cost_calculator_);
  return merchant_ai_->Initialise();
}

util::Status SevenYears::loadShip(const actions::proto::Step& step,
                                  units::Unit* unit) {
  const auto& unit_id = unit->unit_id();
  if (unit->location().has_progress_u() && unit->location().progress_u() != 0) {
    return util::FailedPreconditionError(absl::Substitute(
        "Unit $0 tried to load $1 while in transit: $2, $3",
        util::objectid::DisplayString(unit_id), step.good(),
        unit->location().progress_u(), unit->location().connection_id()));
  }
  const auto& area_id = unit->location().a_area_id();
  auto* area_state = mutable_area_state(area_id);
  if (area_state == nullptr) {
    return util::NotFoundError(
        absl::Substitute("Could not find state for area $0 so $1 could load $2",
                         util::objectid::DisplayString(area_id),
                         util::objectid::DisplayString(unit_id), step.good()));
  }

  auto capacity_u = unit->Capacity(step.good());
  auto available_u = market::GetAmount(area_state->warehouse(), step.good());
  if (available_u > capacity_u) {
    available_u = capacity_u;
  }
  market::Move(step.good(), available_u, area_state->mutable_warehouse(),
               unit->mutable_resources());
  capacity_u -= available_u;
  if (capacity_u <= 0) {
    return util::OkStatus();
  }
  return util::NotComplete();
}

util::Status SevenYears::doEuropeanTrade(const actions::proto::Step& step,
                                         units::Unit* unit) {
  const auto& unit_id = unit->unit_id();
  const auto& area_id = unit->location().a_area_id();
  Log::Debugf("Unit %s doing trade in %s",
              util::objectid::DisplayString(unit_id),
              util::objectid::DisplayString(area_id));
  geography::Area* area = geography::Area::GetById(area_id);
  if (area == nullptr) {
    return util::NotFoundError(absl::Substitute(
        "Could not find area $0", util::objectid::DisplayString(area_id)));
  }
  int bestIndex = -1;
  micro::uMeasure bestAmount = 0;
  for (int i = 0; i < area->num_fields(); ++i) {
    const auto* field = area->field(i);
    Log::Debugf("Field %d has %d import capacity", i,
                market::GetAmount(field->resources(), constants::ImportCapacity()));
    if (field->has_progress()) {
      continue;
    }
    micro::Measure amount =
        market::GetAmount(field->resources(), constants::ImportCapacity());
    if (amount < bestAmount) {
      continue;
    }
    bestAmount = amount;
    bestIndex = i;
  }

  if (bestIndex < 0) {
    return util::NotFoundError(absl::Substitute(
        "Could not find field for $0 to do European trade in $1",
        util::objectid::DisplayString(unit_id),
        util::objectid::DisplayString(area_id)));
  }

  geography::proto::Field* field = area->mutable_field(bestIndex);
  const auto& trade = ProductionChain(constants::EuropeanTrade());
  *field->mutable_progress() = trade.MakeProgress(trade.MaxScaleU());
  // TODO: Calculate this.
  micro::Measure relation_bonus_u = 0;
  // TODO: Don't assume the variant index.
  int var_index = 0;
  auto status =
      trade.PerformStep(field->fixed_capital(), relation_bonus_u, var_index,
                        unit->mutable_resources(), field->mutable_resources(),
                        unit->mutable_resources(), unit->mutable_resources(),
                        field->mutable_progress());
  // TODO: Only do clearing on completion, and if not complete, send error to
  // signal that the ship must stay.
  field->clear_progress();
  if (!status.ok()) {
    return util::FailedPreconditionError(absl::Substitute(
        "Process $0 in field $1 of $2 failed: $3", trade.get_name(), bestIndex,
        util::objectid::DisplayString(area_id),
        status.error_message().as_string()));
  }
  Log::Debugf("%s now has %d supplies", util::objectid::DisplayString(unit_id),
              market::GetAmount(unit->resources(), constants::Supplies()));
  return util::OkStatus();
}

void SevenYears::moveUnits() {
  // Units all plan simultaneously.
  for (auto& unit : game_world_->units_) {
    actions::proto::Strategy* strategy = unit->mutable_strategy();
    if (strategy->strategy_case() ==
        actions::proto::Strategy::STRATEGY_NOT_SET) {
      // TODO: Strategic AI.
      continue;
    }
    actions::proto::Plan* plan = unit->mutable_plan();
    if (plan->steps_size() == 0) {
      auto status = ai::MakePlan(*unit, unit->strategy(), plan);
      if (!status.ok()) {
        Log::Warnf("Could not create plan for unit %s: %s",
                   unit->ID().DebugString(), status.error_message());
        continue;
      }
    }
  }

  for (auto& unit : game_world_->units_) {
    unit->reset_action_points();
  }

  while (true) {
    int count = 0;
    for (auto& unit : game_world_->units_) {
      if (unit->action_points_u() < 1) {
        continue;
      }
      if (unit->plan().steps_size() == 0) {
        continue;
      }
      auto status =
          ai::ExecuteStep(unit->plan(), unit.get());
      if (status.ok()) {
        Log::Debugf("%s completed %s",
                    util::objectid::DisplayString(unit->unit_id()),
                    actions::StepName(unit->plan().steps(0)));
        ai::DeleteStep(unit->mutable_plan());
        count++;
        unit->mutable_plan()->clear_incomplete();
      } else if (util::IsNotComplete(status)) {
        auto inc = unit->plan().incomplete() + 1;
        if (inc > kMaxIncompleteWait) {
          Log::Debugf("%s giving up on plan due to %d incomplete attempts at "
                      "%s, returning to mission pool.",
                      util::objectid::DisplayString(unit->unit_id()), inc,
                      actions::StepName(unit->plan().steps(0)));
          unit->mutable_plan()->clear_steps();
          unit->mutable_plan()->clear_incomplete();
        } else {
          unit->mutable_plan()->set_incomplete(inc);
        }
      } else {
        Log::Debugf("%s could not execute %s: %s",
                    util::objectid::DisplayString(unit->unit_id()),
                    actions::StepName(unit->plan().steps(0)),
                    status.error_message());
      }
    }
    if (count == 0) {
      break;
    }
  }
}
  

void SevenYears::NewTurn() {
  Log::Info("New turn");

  for (const auto& area : game_world_->areas_) {
    area->Update();
    const auto& area_id = area->area_id();
    if (area_states_.find(area_id) == area_states_.end()) {
      Log::Debugf("Could not find state for area %d", area_id.number());
      continue;
    }
    auto& area_state = area_states_.at(area_id);
    for (int i = 0; i < area->num_fields(); ++i) {
      geography::proto::Field* field = area->mutable_field(i);
      if (area_state.production_size() <= i) {
        continue;
      }
      const std::string& chain_name = area_state.production(i);
      const auto& chain = production_chains_[chain_name];

      if (!field->has_progress() || chain.Complete(field->progress())) {
        *field->mutable_progress() = chain.MakeProgress(micro::kOneInU);
      }

      micro::Measure institutional_capital = 0;
      auto status = chain.PerformStep(
          field->fixed_capital(), institutional_capital, 0,
          area_state.mutable_warehouse(), field->mutable_resources(),
          area_state.mutable_warehouse(), area_state.mutable_warehouse(),
          field->mutable_progress());
      if (!status.ok()) {
        Log::Debugf("Could not complete production %s in %s: %s",
                    chain.get_name(), util::objectid::DisplayString(area_id),
                    status.error_message());
        continue;
      }
      if (chain.Complete(field->progress())) {
        field->clear_progress();
      }
    }
  }

  moveUnits();

  dirtyGraphics_ = true;
}

void SevenYears::UpdateGraphicsInfo(interface::Base* gfx) {
  if (!dirtyGraphics_) {
    return;
  }
  std::vector<util::proto::ObjectId> unit_ids;
  for (const auto& unit : game_world_->units_) {
    if (!unit) {
      Log::Warn("Null unit!");
      continue;
    }
    unit_ids.push_back(unit->ID());
  }
  gfx->DisplayUnits(unit_ids);
  dirtyGraphics_ = false;
}

std::vector<std::string>
SevenYears::validation(const games::setup::proto::GameWorld& world) {
  std::vector<std::string> errors;
  for (const auto& unit : world.units()) {
    if (!unit.has_strategy()) {
      continue;
    }
    const auto& strategy = unit.strategy();
    if (strategy.key_case() == actions::proto::Strategy::kLookup) {
      // We need only check the original. If the lookup string is
      // bad the loading code will tell us later.
      continue;
    }

    if (strategy.strategy_case() !=
        actions::proto::Strategy::kSevenYearsMerchant) {
      errors.push_back(
          absl::Substitute("Unit $0 has unsupported strategy case $1",
                           util::objectid::DisplayString(unit.unit_id()),
                           strategy.strategy_case()));
      continue;
    }
    const auto& sym = strategy.seven_years_merchant();
    auto status = merchant_ai_->ValidMission(sym);
    if (!status.ok()) {
      errors.push_back(absl::Substitute(
          "Unit $0: $1", util::objectid::DisplayString(unit.unit_id()),
          status.error_message().ToString()));
      continue;
    }
  }
  return errors;
}

util::Status
SevenYears::LoadScenario(const games::setup::proto::ScenarioFiles& setup) {
  auto status = validateSetup(setup);
  if (!status.ok()) {
    return status;
  }
  Log::Infof("Loaded \"%s\": %s", setup.name(), setup.description());

  games::setup::proto::Scenario scenario_proto;
  status = games::setup::LoadScenario(setup, &scenario_proto);
  if (!status.ok()) {
    return status;
  }

  games::setup::proto::GameWorld world_proto;
  status = games::setup::LoadWorld(setup, &world_proto);
  if (!status.ok()) {
    return status;
  }

  if (!world_proto.HasExtension(sevenyears::proto::WorldState::sevenyears_state)) {
    return util::InvalidArgumentError(
        "World file does not have SevenYears state extension (should open with "
        "\"[sevenyears.proto.WorldState.sevenyears_state] {\".)");
  }

  constants_ = games::setup::Constants(scenario_proto);
  status = games::setup::CanonicaliseWorld(&world_proto);
  if (!status.ok()) {
    return status;
  }

  games::setup::validation::RegisterValidator(
      "SevenYears", [this](const games::setup::proto::GameWorld& gw) {
        return this->validation(gw);
      });
  auto errors = games::setup::validation::Validate(scenario_proto, world_proto);
  if (!errors.empty()) {
    for (const auto& error : errors) {
      Log::Error(error);
    }
    return util::InvalidArgumentError(
        absl::Substitute("Validation error: $0", errors[0]));
  }

  game_world_ = games::setup::World::FromProto(world_proto);
  if (!game_world_) {
    return util::InvalidArgumentError(
        "Failed to create game world from proto.");
  }

  for (int i = 0; i < constants_.production_chains_.size(); ++i) {
    const auto& proto = constants_.production_chains_[i];
    production_chains_.emplace(constants_.production_chains_[i].name(),
                               industry::Production(proto));
  }
  if (production_chains_.empty()) {
    return util::InvalidArgumentError("No production chains found.");
  }

  sevenyears::proto::WorldState* world_state = world_proto.MutableExtension(
      sevenyears::proto::WorldState::sevenyears_state);
  status = validateWorldState(production_chains_, world_state);
  if (!status.ok()) {
    return status;
  }

  for (const auto& ai : world_state->area_states()) {
    area_states_[ai.area_id()] = ai;
  }
  for (const auto& area : game_world_->areas_) {
    const auto& area_id = area->area_id();
    if (area_states_.find(area_id) != area_states_.end()) {
      continue;
    }
    Log::Warnf("Could not find state for area %d",
               util::objectid::DisplayString(area_id));
    sevenyears::proto::AreaState state;
    *state.mutable_area_id() = area_id;
    *state.mutable_owner_id() = util::objectid::kNullId;
    area_states_[area_id] = state;
  }

  return util::OkStatus();
}

const sevenyears::proto::AreaState&
SevenYears::AreaState(const util::proto::ObjectId& area_id) const {
  if (area_states_.find(area_id) == area_states_.end()) {
    Log::Errorf("Could not find state for area %s",
                util::objectid::DisplayString(area_id));
    static sevenyears::proto::AreaState dummy_area_state_;
    return dummy_area_state_;
  }
  return area_states_.at(area_id);
}

sevenyears::proto::AreaState*
SevenYears::mutable_area_state(const util::proto::ObjectId& area_id) {
  if (area_states_.find(area_id) == area_states_.end()) {
    Log::Errorf("Could not find state for area %s",
                util::objectid::DisplayString(area_id));
    static sevenyears::proto::AreaState dummy_area_state_;
    return &dummy_area_state_;
  }
  return &area_states_.at(area_id);
}

const industry::Production&
SevenYears::ProductionChain(const std::string& name) const {
  if (production_chains_.find(name) == production_chains_.end()) {
    Log::Errorf("Could not find production chain %s", name);
    static industry::Production dummy;
    return dummy;
  }
  return production_chains_.at(name);
}


}  // namespace sevenyears
