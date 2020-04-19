#include "games/sevenyears/sevenyears.h"

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/executer.h"
#include "games/ai/planner.h"
#include "games/industry/industry.h"
#include "games/market/goods_utils.h"
#include "games/setup/proto/setup.pb.h"
#include "games/setup/setup.h"
#include "games/setup/validation/validation.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"
#include "util/status/status.h"

namespace sevenyears {

namespace {

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

class SevenYearsMerchant : public ai::UnitAi {
public:
  util::Status AddStepsToPlan(const units::Unit& unit,
                              const actions::proto::Strategy& strategy,
                              actions::proto::Plan* plan) const override;

private:
};

util::Status
SevenYearsMerchant::AddStepsToPlan(const units::Unit& unit,
                                   const actions::proto::Strategy& strategy,
                                   actions::proto::Plan* plan) const {
  if (!strategy.has_seven_years_merchant()) {
    return util::NotFoundError("No SevenYearsMerchant strategy");
  }

  return util::OkStatus();
}

util::Status InitialiseAI() {
  actions::proto::Strategy strategy;
  strategy.mutable_seven_years_merchant()->mutable_base_area_id()->set_number(
      1);
  // This leaks, but no matter, it's max a hundred bytes.
  SevenYearsMerchant* merchant_ai = new SevenYearsMerchant();;
  return ai::RegisterPlanner(strategy, merchant_ai);
}


void SevenYears::NewTurn() {
  Log::Info("New turn");

  const std::string defaultChain = constants_.production_chains_[0].name();
  for (const auto& area : game_world_->areas_) {
    const auto& area_id = area->area_id();
    if (area_states_.find(area_id) == area_states_.end()) {
      Log::Debugf("Could not find state for area %d", area_id.number());
      continue;
    }
    auto& area_state = area_states_.at(area_id);
    for (int i = 0; i < area->num_fields(); ++i) {
      geography::proto::Field* field = area->mutable_field(i);
      std::string chain_name = defaultChain;
      if (area_state.production_size() > i) {
        chain_name = area_state.production(i);
      }
      const auto& chain = production_chains_[chain_name];

      if (!field->has_progress() || chain.Complete(field->progress())) {
        *field->mutable_progress() = chain.MakeProgress(micro::kOneInU);
      }

      market::Measure institutional_capital = 0;
      if (!chain.PerformStep(
              field->fixed_capital(), institutional_capital, 0,
              area_state.mutable_warehouse(), field->mutable_resources(),
              area_state.mutable_warehouse(), area_state.mutable_warehouse(),
              field->mutable_progress())) {
        continue;
      }
      if (chain.Complete(field->progress())) {
        field->clear_progress();
      }
    }
  }

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

  // Execute in single steps.
  for (int i = 0; i < 3; ++i) {
    for (auto& unit : game_world_->units_) {
      auto status = ai::ExecuteStep(unit->plan(), unit.get());
      if (status.ok()) {
        ai::DeleteStep(unit->mutable_plan());
      } else {
        Log::Warnf("Could not execute unit plan: %s", status.error_message());
      }
    }
  }

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
    //production_chains_[proto.name()] =
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

  return util::OkStatus();
}

}  // namespace sevenyears
