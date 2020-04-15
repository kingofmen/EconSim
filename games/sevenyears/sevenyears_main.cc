#include <experimental/filesystem>
#include <fstream>
#include <vector>

#include "absl/strings/substitute.h"
#include "games/actions/proto/plan.pb.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/ai/executer.h"
#include "games/ai/planner.h"
#include "games/setup/proto/setup.pb.h"
#include "games/setup/setup.h"
#include "games/interface/base.h"
#include "games/interface/proto/config.pb.h"
#include "games/sevenyears/graphics/sdl_interface.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/logging/logging.h"
#include "util/proto/file.h"
#include "util/status/status.h"

#include "SDL.h"

google::protobuf::util::Status
validateSetup(const games::setup::proto::ScenarioFiles& setup) {
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

std::vector<std::experimental::filesystem::path> getScenarioPaths() {
  auto current_path = std::experimental::filesystem::current_path();
  current_path /= "scenarios";

  if (!std::experimental::filesystem::exists(current_path)) {
    return {};
  }
  auto file_it = std::experimental::filesystem::directory_iterator(current_path);
  auto end = std::experimental::filesystem::directory_iterator();
  std::vector<std::experimental::filesystem::path> scenarios;
  for (; file_it != end; ++file_it) {
    if (file_it->path().extension() != ".scenario") {
      continue;
    }
    scenarios.push_back(file_it->path());
  }

  return scenarios;
}

std::vector<games::setup::proto::ScenarioFiles>
getScenarios(const std::vector<std::experimental::filesystem::path> paths) {
  std::vector<games::setup::proto::ScenarioFiles> scenarios;
  for (const auto& path : paths) {
    scenarios.emplace_back();
    auto status = util::proto::ParseProtoFile(path.string(), &scenarios.back());
    if (!status.ok()) {
      Log::Errorf("Error reading file %s: %s", path.filename().string(), status.error_message());
      scenarios.pop_back();
      continue;
    }
    if (scenarios.back().name().empty()) {
      scenarios.back().set_name(path.filename().string());
    }
    if (!scenarios.back().has_root_path()) {
      scenarios.back().set_root_path(path.parent_path().string());
    }    
  }
  return scenarios;
}

sevenyears::graphics::SevenYearsInterface* createInterface() {
  return new sevenyears::graphics::SDLInterface();
}

util::Status
loadGraphicsInfo(sevenyears::graphics::SevenYearsInterface* interface) {
  auto exe_path = std::experimental::filesystem::current_path();
  auto graphics_path = exe_path / "gfx/dev_graphics.pb.txt";
  if (!std::experimental::filesystem::exists(graphics_path)) {
    return util::NotFoundError(absl::Substitute(
        "Could not find scenario graphics file $0", graphics_path.string()));
  }

  sevenyears::graphics::proto::Scenario graphics;
  auto status = util::proto::ParseProtoFile(graphics_path.string(), &graphics);
  if (!status.ok()) {
    return status;
  }

  return interface->ScenarioGraphics(graphics);
}

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

// Class for running actual game mechanics.
class SevenYears {
public:
  SevenYears() : dirtyGraphics_(true) {}
  ~SevenYears() {}

  util::Status LoadScenario(const games::setup::proto::ScenarioFiles& setup);
  void NewTurn();
  void UpdateGraphicsInfo(interface::Base* gfx);

private:
  bool dirtyGraphics_;
  std::unique_ptr<games::setup::World> game_world_;
  games::setup::Constants constants_;
  std::unordered_map<std::string, int> chain_indices_;
  std::unordered_map<util::proto::ObjectId, sevenyears::proto::AreaInfo>
      area_infos_;
};

void SevenYears::NewTurn() {
  Log::Info("New turn");

  const std::string defaultChain = constants_.production_chains_[0].name();
  for (const auto& area : game_world_->areas_) {
    const auto& area_id = area->area_id();
    if (area_infos_.find(area_id) == area_infos_.end()) {
      Log::Debugf("Could not find info for area %d", area_id.number());
      continue;
    }
    const auto& area_info = area_infos_.at(area_id);
    for (int i = 0; i < area->num_fields(); ++i) {
      geography::proto::Field* field = area->mutable_field(i);
      std::string chain = defaultChain;
      if (area_info.production_size() > i) {
        chain = area_info.production(i);
      }
      Log::Infof("Area %d field %d running chain %s", area_id.number(), i,
                 chain);
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
  while (true) {
    int count = 0;
    for (auto& unit : game_world_->units_) {
      if (ai::ExecuteStep(unit->plan(), unit.get())) {
        ai::DeleteStep(unit->mutable_plan());
        ++count;
      }
    }
    if (count == 0) {
      break;
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
  constants_ = games::setup::Constants(scenario_proto);

  games::setup::proto::GameWorld world_proto;
  status = games::setup::LoadWorld(setup, &world_proto);
  if (!status.ok()) {
    return status;
  }
  status = games::setup::CanonicaliseWorld(&world_proto);
  if (!status.ok()) {
    return status;
  }
  game_world_ = games::setup::World::FromProto(world_proto);
  if (!game_world_) {
    return util::InvalidArgumentError(
        "Failed to create game world from proto.");
  }

  for (int i = 0; i < constants_.production_chains_.size(); ++i) {
    chain_indices_[constants_.production_chains_[i].name()] = i;
  }
  if (chain_indices_.empty()) {
    return util::InvalidArgumentError("No production chains found.");
  }

  sevenyears::proto::WorldState world_state;
  std::unordered_map<std::string, google::protobuf::Message*> locations = {
      {"area_info", &world_state}};
  status = games::setup::LoadExtras(setup, locations);
  if (!status.ok()) {
    return status;
  }

  for (const auto& ai : world_state.area_infos()) {
    area_infos_[ai.area_id()] = ai;
  }
  
  return util::OkStatus();
}

class EventHandler : public interface::Receiver {
public:
  EventHandler(SevenYears* game) : game_(game), quit_(false) {}

  void QuitToDesktop() override {
    quit_ = true;
  }

  bool quit() const { return quit_; }

  void HandleKeyRelease(const SDL_Keysym& keysym);

private:
  bool quit_;
  SevenYears* game_;
};

void EventHandler::HandleKeyRelease(const SDL_Keysym& keysym) {
  switch (keysym.sym) {
    case SDLK_KP_ENTER:
    case SDLK_RETURN:
      game_->NewTurn();
      break;
    case SDLK_q:
      quit_ = true;
      break;
    default:
      break;
  }
}

util::Status InitialiseAI() {
  actions::proto::Strategy strategy;
  strategy.mutable_seven_years_merchant()->mutable_base_area_id()->set_number(
      1);
  // This leaks, but no matter, it's max a hundred bytes.
  SevenYearsMerchant* merchant_ai = new SevenYearsMerchant();;
  return ai::RegisterPlanner(strategy, merchant_ai);
}

int main(int /*argc*/, char** /*argv*/) {
  Log::Register(Log::coutLogger);
  auto paths = getScenarioPaths();
  if (paths.empty()) {
    Log::Error("No scenarios found.");
    return 1;
  }

  auto scenarios = getScenarios(paths);
  if (scenarios.empty()) {
    Log::Error("Could not parse any scenario files.");
    return 2;
  }

  SevenYears sevenYears;
  if (scenarios.size() == 1) {
    auto status = sevenYears.LoadScenario(scenarios[0]);
    if (!status.ok()) {
      Log::Errorf("Error loading scenario: %s", status.error_message());
      return 3;
    }
  }
  Log::Infof("Created world");

  auto status = InitialiseAI();
  if (!status.ok()) {
    Log::Errorf("Error initialising AI: %s", status.error_message());
    return 3;
  }
  
  games::interface::proto::Config config;
  config.set_screen_size(games::interface::proto::Config::SS_1440_900);
  EventHandler handler(&sevenYears);

  sevenyears::graphics::SevenYearsInterface* graphics = createInterface();
  graphics->SetReceiver(&handler);

  status = graphics->Initialise(config);
  if (!status.ok()) {
    Log::Errorf("Error initialising interface: %s", status.error_message());
    graphics->Cleanup();
    return 4;
  }

  status = loadGraphicsInfo(graphics);
  if (!status.ok()) {
    Log::Errorf("Error loading graphics info: %s", status.error_message());
    graphics->Cleanup();
    return 5;
  }

  while (!handler.quit()) {
    graphics->EventLoop();
    sevenYears.UpdateGraphicsInfo(graphics);
  }

  graphics->Cleanup();

  return 0;
}
