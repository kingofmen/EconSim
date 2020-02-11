#include "games/sevenyears/graphics/sdl_interface.h"

#include "absl/strings/substitute.h"
#include "interface/proto/config.pb.h"
#include "util/status/status.h"
#include "SDL.h"


namespace sevenyears {
namespace graphics {
namespace {

void widthAndHeight(const interface::proto::Config::ScreenSize& ss, int& width, int& height) {
  switch (ss) {
    case interface::proto::Config::SS_1280_800:
      width = 1280;
      height = 800;
      break;
    case interface::proto::Config::SS_1280_1024:
      width = 1280;
      height = 1024;
      break;
    case interface::proto::Config::SS_1366_768:
      width = 1366;
      height = 768;
      break;
    case interface::proto::Config::SS_1440_900:
      width = 1440;
      height = 900;
      break;
    case interface::proto::Config::SS_1600_900:
      width = 1600;
      height = 900;
      break;
    case interface::proto::Config::SS_1920_1080:
      width = 1920;
      height = 1080;
      break;
      
    case interface::proto::Config::SS_DEFAULT:
    default:
      break;
  }
}

util::Status validate(const proto::Scenario& scenario) {
  if (scenario.maps().empty()) {
    return util::NotFoundError("Scenario has no maps.");
  }

  int counter = 0;
  for (const auto& map : scenario.maps()) {
    counter++;
    if (map.filename().empty()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no filename.", counter));
    }
    if (map.area_ids().empty()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no areas.", map.filename()));
    }
  }

  return util::OkStatus();
}


}  // namespace


util::Status SDLInterface::Initialise(const interface::proto::Config& config) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not initialise SDL: $0", SDL_GetError()));
  }

  int width = 640;
  int height = 480;
  widthAndHeight(config.screen_size(), width, height);
  window_.reset(SDL_CreateWindow("Seven Years", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, width, height,
                                 SDL_WINDOW_SHOWN));
  if (!window_) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not create window: $0", SDL_GetError()));
  }

  SDL_Surface* screenSurface = SDL_GetWindowSurface(window_.get());
  SDL_FillRect(screenSurface, NULL,
               SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
  SDL_UpdateWindowSurface(window_.get());
  return util::OkStatus();
}

void SDLInterface::Cleanup() {
  window_.reset(nullptr); // Also calls deleter.
  SDL_Quit();
}

void SDLInterface::EventLoop() {
  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    if (e.type == SDL_QUIT) {
      receiver_->QuitToDesktop();
    }
  }
}

util::Status SDLInterface::ScenarioGraphics(const proto::Scenario& scenario) {
  auto status = validate(scenario);
  if (!status.ok()) {
    return status;
  }
  
  return util::OkStatus();
}

}  // namespace graphics
}  // namespace sevenyears