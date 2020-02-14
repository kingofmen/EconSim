#include "games/sevenyears/graphics/sdl_interface.h"

#include <experimental/filesystem>
#include <utility>

#include "absl/strings/substitute.h"
#include "interface/proto/config.pb.h"
#include "games/sevenyears/graphics/bitmap.h"
#include "util/status/status.h"
#include "SDL.h"

#include "util/logging/logging.h"

namespace sevenyears {
namespace graphics {
namespace {

int seconds(const proto::Coord& coord) {
  return 3600 * coord.degrees() + 60 * coord.minutes() + coord.seconds() +
         coord.adjust();
}

int seconds_north(const proto::LatLong& coord) {
  if (coord.has_north()) {
    return seconds(coord.north());
  }
  return -seconds(coord.south());
}

int seconds_east(const proto::LatLong& coord) {
  if (coord.has_east()) {
    return seconds(coord.east());
  }
  return -seconds(coord.west());
}

void widthAndHeight(const interface::proto::Config::ScreenSize& ss, int& width,
                    int& height, SDL_Rect* map_rectangle) {
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

  map_rectangle->h = height;
  // TODO: Actually derive some values for this.
  map_rectangle->w = 792;
}

util::Status validate(const proto::Scenario& scenario) {
  if (scenario.maps().empty()) {
    return util::NotFoundError("Scenario has no maps.");
  }

  if (!scenario.has_root_gfx_path() || scenario.root_gfx_path().empty()) {
    return util::NotFoundError("Scenario has no root graphics path.");
  }

  int counter = 0;
  std::unordered_map<int, std::string> area_ids;
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
    if (!map.has_left_top()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no left-top coordinate.", map.name()));
    }
    if (!map.has_right_bottom()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no right-bottom coordinate.", map.name()));
    }
    if (seconds_north(map.left_top()) <= seconds_north(map.right_bottom())) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has left top south of right bottom.", map.name()));
    }
    if (seconds_east(map.left_top()) >= seconds_east(map.right_bottom())) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has left top east of right bottom.", map.name()));
    }
    if (map.areas().empty()) {
      return util::InvalidArgumentError(
          absl::Substitute("Map $0 has no areas.", map.name()));
    }

    int top_seconds = seconds_north(map.left_top());
    int left_seconds = seconds_east(map.left_top());
    int bottom_seconds = seconds_north(map.right_bottom());
    int right_seconds = seconds_east(map.right_bottom());
    for (const auto& area : map.areas()) {
      if (!area.has_area_id()) {
        continue;
      }
      int curr_id = area.area_id();
      if (area_ids.find(curr_id) != area_ids.end()) {
        return util::InvalidArgumentError(
            absl::Substitute("Duplicate area $0 in map $1, previous was in $2",
                             curr_id, map.name(), area_ids[curr_id]));
      }
      area_ids[curr_id] = map.name();

      if (!area.has_position()) {
        return util::InvalidArgumentError(absl::Substitute(
            "Area $0 in map $1 has no coordinates.", curr_id, map.name()));
      }
      const auto& pos = area.position();
      if (seconds_north(pos) > top_seconds) {
        return util::InvalidArgumentError(
            absl::Substitute("Area $0 in map $1 is north of the map top edge.",
                             curr_id, map.name()));
      }
      if (seconds_east(pos) < left_seconds) {
        return util::InvalidArgumentError(
            absl::Substitute("Area $0 in map $1 is west of the map left edge.",
                             curr_id, map.name()));
      }
      if (seconds_north(pos) < bottom_seconds) {
        return util::InvalidArgumentError(absl::Substitute(
            "Area $0 in map $1 is south of the map bottom edge.", curr_id,
            map.name()));
      }
      if (seconds_east(pos) > right_seconds) {
        return util::InvalidArgumentError(
            absl::Substitute("Area $0 in map $1 is east of the map right edge.",
                             curr_id, map.name()));
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
  widthAndHeight(config.screen_size(), width, height, &map_rectangle_);
  window_.reset(SDL_CreateWindow("Seven Years", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, width, height,
                                 SDL_WINDOW_SHOWN));
  if (!window_) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not create window: $0", SDL_GetError()));
  }

  renderer_.reset(
      SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED));
  if (!renderer_) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not create renderer: $0", SDL_GetError()));
  }

  clearScreen();
  SDL_UpdateWindowSurface(window_.get());
  return util::OkStatus();
}

void SDLInterface::Cleanup() {
  for (auto map : maps_) {
    SDL_DestroyTexture(map.second.background_);
  }
  window_.reset(nullptr); // Also calls deleter.
  renderer_.reset(nullptr);
  SDL_Quit();
}

void SDLInterface::clearScreen() {
  SDL_SetRenderDrawColor(renderer_.get(), 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderClear(renderer_.get());
}

void SDLInterface::drawArea(const Area& area) {
  SDL_Rect fillRect = {area.xpos_ - 5, area.ypos_ - 5, 10, 10};
  SDL_SetRenderDrawColor(renderer_.get(), 0xFF, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer_.get(), &fillRect);
}

void SDLInterface::drawMap() {
  clearScreen();
  if (current_map_.empty()) {
    return;
  }
  const Map& currMap = maps_.at(current_map_);
  SDL_RenderCopy(renderer_.get(), currMap.background_, NULL, &map_rectangle_);
  for (const Area& area : currMap.areas_) {
    drawArea(area);
  }
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
  SDL_RenderPresent(renderer_.get());
}

// TODO: Ok get this in a library, with the other obvious operators, and unit
// tests.
proto::LatLong operator-(const proto::LatLong& lhs, const proto::LatLong& rhs) {
  int seconds = 0;
  if (lhs.has_north()) {
    seconds += lhs.north().seconds();
    seconds += lhs.north().minutes() * 60;
    seconds += lhs.north().degrees() * 60 * 60;
  }
  if (lhs.has_south()) {
    seconds -= lhs.south().seconds();
    seconds -= lhs.south().minutes() * 60;
    seconds -= lhs.south().degrees() * 60 * 60;
  }

  proto::LatLong ret;
  proto::Coord* target = NULL;
  if (seconds >= 0) {
    target = ret.mutable_north();
  } else {
    target = ret.mutable_south();
    seconds *= -1;
  }

  target->set_degrees(seconds / 3600);
  seconds -= target->degrees() * 3600;
  target->set_minutes(seconds / 60);
  seconds -= target->minutes() * 60;
  target->set_seconds(seconds);

  seconds = 0;
  if (lhs.has_east()) {
    seconds += lhs.east().seconds();
    seconds += lhs.east().minutes() * 60;
    seconds += lhs.east().degrees() * 60 * 60;
  }
  if (lhs.has_west()) {
    seconds -= lhs.west().seconds();
    seconds -= lhs.west().minutes() * 60;
    seconds -= lhs.west().degrees() * 60 * 60;
  }

  if (seconds >= 0) {
    target = ret.mutable_east();
  } else {
    target = ret.mutable_west();
    seconds *= -1;
  }

  target->set_degrees(seconds / 3600);
  seconds -= target->degrees() * 3600;
  target->set_minutes(seconds / 60);
  seconds -= target->minutes() * 60;
  target->set_seconds(seconds);

  return ret;
}

SDLInterface::Area::Area(const proto::Area& proto,
                         const proto::LatLong& topleft,
                         int seconds_per_pixel_high,
                         int seconds_per_pixel_wide) {
  int seconds = seconds_north(topleft) - seconds_north(proto.position());
  ypos_ = seconds / seconds_per_pixel_high;
  seconds = seconds_east(proto.position()) - seconds_east(topleft);
  xpos_ = seconds / seconds_per_pixel_wide;
}

SDLInterface::Map::Map(const proto::Map& proto) : background_(NULL) {
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
    if (maps_.find(map.name()) != maps_.end()) {
      return util::InvalidArgumentError(
          absl::Substitute("Duplicate map name $0", map.name()));
    }

    Map curr(map);
    auto current_path = base_path / map.filename();
    SDL_Surface* surface = NULL;
    status = bitmap::LoadForSdl(current_path, surface);
    if (!status.ok()) {
      return status;
    }
    curr.background_ = SDL_CreateTextureFromSurface(renderer_.get(), surface);
    if (!curr.background_) {
      return util::InvalidArgumentError(
          absl::Substitute("Could not convert $0 to texture: $1",
                           current_path.string(), SDL_GetError()));
    }
    SDL_FreeSurface(surface);

    int seconds_per_pixel_high =
        seconds_north(map.left_top()) - seconds_north(map.right_bottom());
    seconds_per_pixel_high /= map_rectangle_.h;
    int seconds_per_pixel_wide =
        seconds_east(map.right_bottom()) - seconds_east(map.left_top());
    seconds_per_pixel_wide /= map_rectangle_.w;
    for (const auto& area_proto : map.areas()) {
      curr.areas_.emplace_back(area_proto, map.left_top(),
                               seconds_per_pixel_high, seconds_per_pixel_wide);
    }
    maps_.emplace(map.name(), std::move(curr));
    current_map_ = map.name();
  }

  return util::OkStatus();
}

}  // namespace graphics
}  // namespace sevenyears
