#include "games/sevenyears/graphics/sdl_interface.h"

#include <experimental/filesystem>

#include "absl/strings/substitute.h"
#include "interface/proto/config.pb.h"
#include "games/sevenyears/graphics/bitmap.h"
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

  if (!scenario.has_root_gfx_path() || scenario.root_gfx_path().empty()) {
    return util::NotFoundError("Scenario has no root graphics path.");
  }

  int counter = 0;
  for (const auto& map : scenario.maps()) {
    counter++;
    if (map.name().empty()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no name.", counter));
    }
    if (map.filename().empty()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no filename.", map.name()));
    }
    if (!map.has_left_top_corner()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no left-top coordinate.", map.name()));
    }
    if (map.areas().empty()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no areas.", map.name()));
    }
    for (const auto& area : map.areas()) {
      if (!area.has_area_id()) {
        continue;
      }
      if (!area.has_position()) {
        return util::InvalidArgumentError(
            absl::Substitute("Area $0 in map $1 has no coordinates.",
                             area.area_id(), map.name()));
      }
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

  clearScreen();
  SDL_UpdateWindowSurface(window_.get());
  return util::OkStatus();
}

void SDLInterface::Cleanup() {
  for (auto map : maps_) {
    SDL_FreeSurface(map.second);
  }
  window_.reset(nullptr); // Also calls deleter.
  SDL_Quit();
}

void SDLInterface::clearScreen() {
  SDL_Surface* screenSurface = SDL_GetWindowSurface(window_.get());
  SDL_FillRect(screenSurface, NULL,
               SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
}

void SDLInterface::drawMap() {
  clearScreen();
  if (current_map_.empty()) {
    return;
  }
  SDL_Surface* screen = SDL_GetWindowSurface(window_.get());
  SDL_BlitSurface(maps_[current_map_], NULL, screen, NULL);
}

void SDLInterface::EventLoop() {
  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    if (e.type == SDL_QUIT) {
      receiver_->QuitToDesktop();
    }
  }

  // TODO: Put this in a separate thread, like a big boy.
  drawMap();
  SDL_UpdateWindowSurface(window_.get());
}

util::Status SDLInterface::ScenarioGraphics(const proto::Scenario& scenario) {
  auto status = validate(scenario);
  if (!status.ok()) {
    return status;
  }

  auto base_path = std::experimental::filesystem::current_path();
  base_path /= scenario.root_gfx_path();
  if (!std::experimental::filesystem::exists(base_path)) {
    return util::NotFoundError(absl::Substitute(
        "Could not find base graphics path $0", base_path.string()));
  }

  for (const proto::Map& map : scenario.maps()) {
    auto current_path = base_path / map.filename();
    SDL_Surface* curr = NULL;
    status = bitmap::LoadForSdl(current_path, curr);
    if (!status.ok()) {
      return status;
    }
    if (maps_.find(map.name()) != maps_.end()) {
      return util::InvalidArgumentError(
          absl::Substitute("Duplicate map $0", map.name()));
    }
    maps_[map.name()] = curr;
    current_map_ = map.name();
  }

  return util::OkStatus();
}

}  // namespace graphics
}  // namespace sevenyears
