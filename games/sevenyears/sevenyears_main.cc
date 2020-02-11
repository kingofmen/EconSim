#include <experimental/filesystem>
#include <fstream>
#include <vector>

#include "absl/strings/substitute.h"
#include "game/setup/proto/setup.pb.h"
#include "game/proto/game_world.pb.h"
#include "game/validation/validation.h"
#include "interface/base.h"
#include "interface/proto/config.pb.h"
#include "games/sevenyears/graphics/sdl_interface.h"
#include "util/logging/logging.h"
#include "util/proto/file.h"
#include "util/status/status.h"

#include "SDL.h"

google::protobuf::util::Status
validateSetup(const game::setup::proto::ScenarioFiles& setup) {
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

std::vector<game::setup::proto::ScenarioFiles>
getScenarios(const std::vector<std::experimental::filesystem::path> paths) {
  std::vector<game::setup::proto::ScenarioFiles> scenarios;
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

google::protobuf::util::Status
loadScenario(const game::setup::proto::ScenarioFiles& setup) {
  auto status = validateSetup(setup);
  if (!status.ok()) {
    return status;
  }
  Log::Infof("Loaded \"%s\": %s", setup.name(), setup.description());

  std::experimental::filesystem::path base_path = setup.root_path();
  game::proto::GameWorld game_world;
  std::experimental::filesystem::path world_path = base_path / setup.world_file();
  status = util::proto::ParseProtoFile(world_path.string(), &game_world);
  if (!status.ok()) {
    return status;
  }

  std::vector<std::string> errors = game::validation::Validate({}, game_world);
  if (!errors.empty()) {
    for (const auto err : errors) {
      Log::Error(err);
    }
    return util::InvalidArgumentError("Validation errors in scenario");
  }

  return util::OkStatus();
}

sevenyears::graphics::SevenYearsInterface* createInterface() {
  return new sevenyears::graphics::SDLInterface();
}

class EventHandler : public interface::Receiver {
public:
  EventHandler() : quit_(false) {}

  void QuitToDesktop() override {
    quit_ = true;
  }

  bool quit() const { return quit_; }

private:
  bool quit_;
};

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

  if (scenarios.size() == 1) {
    auto status = loadScenario(scenarios[0]);
    if (!status.ok()) {
      Log::Errorf("Error loading scenario: %s", status.error_message());
      return 3;
    }
  }

  interface::proto::Config config;
  config.set_screen_size(interface::proto::Config::SS_1440_900);
  EventHandler handler;

  interface::Base* graphics = createInterface();
  graphics->SetReceiver(&handler);

  auto status = graphics->Initialise(config);
  if (!status.ok()) {
    Log::Errorf("Error creating graphics: %s", status.error_message());
    return 4;
  }

  while (!handler.quit()) {
    graphics->EventLoop();
  }

  graphics->Cleanup();

  return 0;
}
