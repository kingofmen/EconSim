#include <experimental/filesystem>
#include <fstream>
#include <vector>

#include "absl/strings/substitute.h"
#include "games/setup/proto/setup.pb.h"
#include "games/setup/setup.h"
#include "games/interface/base.h"
#include "games/interface/proto/config.pb.h"
#include "games/sevenyears/graphics/sdl_interface.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "games/sevenyears/sevenyears.h"
#include "util/logging/logging.h"
#include "util/proto/file.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

#include "SDL.h"

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

class EventHandler : public interface::Receiver {
public:
  EventHandler(sevenyears::SevenYears* game) : game_(game), quit_(false) {}

  void QuitToDesktop() override {
    quit_ = true;
  }

  bool quit() const { return quit_; }

  void HandleKeyRelease(const SDL_Keysym& keysym) override;
  void HandleMouseEvent(const interface::MouseClick& mc) override;
  void SelectObject(const util::proto::ObjectId& object_id) override;

private:
  bool quit_;
  sevenyears::SevenYears* game_;
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

void EventHandler::HandleMouseEvent(const interface::MouseClick& mc) {}

void EventHandler::SelectObject(const util::proto::ObjectId& object_id) {
  if (util::objectid::IsNull(object_id)) {
    return;
  }
  Log::Infof("Selected object %s", util::objectid::DisplayString(object_id));
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

  sevenyears::SevenYears sevenYears;
  auto status = sevenYears.InitialiseAI();
  if (!status.ok()) {
    Log::Errorf("Error initialising AI: %s", status.error_message());
    return 3;
  }
  Log::Infof("Initialised AI");

  if (scenarios.size() == 1) {
    status = sevenYears.LoadScenario(scenarios[0]);
    if (!status.ok()) {
      Log::Errorf("Error loading scenario: %s", status.error_message());
      return 3;
    }
  }
  Log::Infof("Created world");

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
