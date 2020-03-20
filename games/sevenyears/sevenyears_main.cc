#include <experimental/filesystem>
#include <fstream>
#include <vector>

#include "absl/strings/substitute.h"
#include "games/setup/proto/setup.pb.h"
#include "games/setup/setup.h"
#include "games/setup/validation/validation.h"
#include "interface/base.h"
#include "interface/proto/config.pb.h"
#include "games/sevenyears/graphics/sdl_interface.h"
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

util::Status loadScenario(const games::setup::proto::ScenarioFiles& setup) {
  auto status = validateSetup(setup);
  if (!status.ok()) {
    return status;
  }
  Log::Infof("Loaded \"%s\": %s", setup.name(), setup.description());

  games::setup::proto::GameWorld game_world;
  status = games::setup::LoadWorld(setup, &game_world);
  if (!status.ok()) {
    return status;
  }
  games::setup::proto::Scenario scenario;
  status = games::setup::LoadScenario(setup, &scenario);
  if (!status.ok()) {
    return status;
  }

  std::vector<std::string> errors =
      games::setup::validation::Validate(scenario, game_world);
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

class EventHandler : public interface::Receiver {
public:
  EventHandler() : quit_(false) {}

  void QuitToDesktop() override {
    quit_ = true;
  }

  bool quit() const { return quit_; }

  void HandleKeyRelease(const SDL_Keysym& keysym);

private:
  bool quit_;
};

void EventHandler::HandleKeyRelease(const SDL_Keysym& keysym) {
  switch (keysym.sym) {
    case SDLK_KP_ENTER:
    case SDLK_RETURN:
      Log::Info("New turn");
      break;
    case SDLK_q:
      quit_ = true;
      break;
    default:
      break;
  }
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

  sevenyears::graphics::SevenYearsInterface* graphics = createInterface();
  graphics->SetReceiver(&handler);

  auto status = graphics->Initialise(config);
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
  }

  graphics->Cleanup();

  return 0;
}
