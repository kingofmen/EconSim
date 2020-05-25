#include "games/sevenyears/graphics/sdl_interface.h"

#include <experimental/filesystem>
#include <utility>

#include "absl/strings/substitute.h"
#include "games/interface/proto/config.pb.h"
#include "games/sevenyears/graphics/bitmap.h"
#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

#include "SDL.h"
#include "SDL_ttf.h"

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

void widthAndHeight(const games::interface::proto::Config::ScreenSize& ss,
                    int& width, int& height, SDL_Rect* map_rectangle) {
  switch (ss) {
    case games::interface::proto::Config::SS_1280_800:
      width = 1280;
      height = 800;
      break;
    case games::interface::proto::Config::SS_1280_1024:
      width = 1280;
      height = 1024;
      break;
    case games::interface::proto::Config::SS_1366_768:
      width = 1366;
      height = 768;
      break;
    case games::interface::proto::Config::SS_1440_900:
      width = 1440;
      height = 900;
      break;
    case games::interface::proto::Config::SS_1600_900:
      width = 1600;
      height = 900;
      break;
    case games::interface::proto::Config::SS_1920_1080:
      width = 1920;
      height = 1080;
      break;
      
    case games::interface::proto::Config::SS_DEFAULT:
    default:
      break;
  }

  map_rectangle->h = height;
  // TODO: Actually derive some values for this.
  map_rectangle->w = 792;
}


}  // namespace

void SDLInterface::Cleanup() {
  sprites_->Cleanup();
  TTF_Quit();
  SDL_Quit();
}

util::Status
SDLInterface::Initialise(const games::interface::proto::Config& config) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not initialise SDL: $0", SDL_GetError()));
  }
  auto ttf = TTF_Init();
  if (ttf < 0) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not initialise TTF: $0", ttf));
  }

  int width = 640;
  int height = 480;
  widthAndHeight(config.screen_size(), width, height, &map_rectangle_);
  sprites_ = new SDLSpriteDrawer();
  sprites_->Init(width, height);
  sprites_->ClearScreen();
  sprites_->Update();
  return util::OkStatus();
}

void SDLInterface::drawMap() {
  sprites_->ClearScreen();
  if (current_map_.empty()) {
    return;
  }
  const Map& currMap = maps_.at(current_map_);
  sprites_->DrawMap(currMap, &map_rectangle_);
  sprites_->Update();
}

util::Status SDLInterface::validate(const proto::Scenario& scenario) {
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
      const util::proto::ObjectId& curr_id = area.area_id();
      if (area_map_.find(curr_id) != area_map_.end()) {
        return util::InvalidArgumentError(absl::Substitute(
            "Duplicate area $0 in map $1, previous was in $2",
            curr_id.DebugString(), map.name(), area_map_[curr_id].first));
      }
      area_map_.emplace(std::make_pair(curr_id, std::make_pair(map.name(), 0)));

      if (!area.has_position()) {
        return util::InvalidArgumentError(
            absl::Substitute("Area $0 in map $1 has no coordinates.",
                             curr_id.DebugString(), map.name()));
      }
      const auto& pos = area.position();
      if (seconds_north(pos) > top_seconds) {
        return util::InvalidArgumentError(
            absl::Substitute("Area $0 in map $1 is north of the map top edge.",
                             curr_id.DebugString(), map.name()));
      }
      if (seconds_east(pos) < left_seconds) {
        return util::InvalidArgumentError(
            absl::Substitute("Area $0 in map $1 is west of the map left edge.",
                             curr_id.DebugString(), map.name()));
      }
      if (seconds_north(pos) < bottom_seconds) {
        return util::InvalidArgumentError(absl::Substitute(
            "Area $0 in map $1 is south of the map bottom edge.",
            curr_id.DebugString(), map.name()));
      }
      if (seconds_east(pos) > right_seconds) {
        return util::InvalidArgumentError(
            absl::Substitute("Area $0 in map $1 is east of the map right edge.",
                             curr_id.DebugString(), map.name()));
      }
    }
  }

  if (scenario.fonts().empty()) {
    return util::InvalidArgumentError("Scenario has no fonts.");
  }

  return util::OkStatus();
}

void SDLInterface::DisplayUnits(const std::vector<util::proto::ObjectId>& ids) {
  for (auto& cm : maps_) {
    cm.second.unit_locations_.clear();
    for (auto& area : cm.second.areas_) {
      area.unit_numbers_.clear();
    }
  }
  for (const auto& unit_id : ids) {
    units::Unit* unit = units::Unit::ById(unit_id);
    if (unit == NULL) {
      continue;
    }
    const auto& location = unit->location();
    if (location.has_connection_id()) {
      const auto* connection =
          geography::Connection::ById(location.connection_id());
      if (connection == NULL) {
        continue;
      }
      const util::proto::ObjectId& a_id = location.a_area_id();
      const util::proto::ObjectId& z_id = connection->OtherSide(a_id);
      double a_weight = location.progress_u();
      a_weight /= connection->length_u();
      // TODO: Handle connections between different maps.
      if (area_map_[a_id].first != area_map_[z_id].first) {
        continue;
      }
      if (maps_.find(area_map_[a_id].first) == maps_.end()) {
        Log::Debugf("Could not find area %d: %s", a_id.DebugString(),
                    area_map_[a_id].first);
        continue;
      }
      Map& currMap = maps_.at(area_map_[a_id].first);
      double xpos = 0;
      double ypos = 0;
      for (const auto& area : currMap.areas_) {
        if (area.area_id_ == a_id) {
          xpos += area.xpos_ * a_weight;
          ypos += area.ypos_ * a_weight;
        } else if (area.area_id_ == z_id) {
          xpos += area.xpos_ * (1 - a_weight);
          ypos += area.ypos_ * (1 - a_weight);
        }
      }
      currMap.unit_locations_[unit_id] = {(int)floor(xpos + 0.5),
                                          (int)floor(ypos + 0.5), 16, 16};
    } else {
      if (area_map_.find(location.a_area_id()) == area_map_.end()) {
        continue;
      }
      Area& area = getAreaById(location.a_area_id());
      area.unit_numbers_[unit->template_kind()]++;
    }
  }
}

Area& SDLInterface::getAreaById(const util::proto::ObjectId& area_id) {
  const auto& area_idx = area_map_[area_id];
  return maps_.at(area_idx.first).areas_[area_idx.second];
}

interface::MouseClick makeMouseClick(const SDL_MouseButtonEvent& event) {
  interface::MouseClick click;
  switch (event.button) {
    case SDL_BUTTON_LEFT:
    default:
      click.button_ = interface::MouseClick::MB_LEFT;
      break;
    case SDL_BUTTON_RIGHT:
      click.button_ = interface::MouseClick::MB_RIGHT;
      break;
    case SDL_BUTTON_MIDDLE:
      click.button_ = interface::MouseClick::MB_MIDDLE;
      break;
    case SDL_BUTTON_X1:
      click.button_ = interface::MouseClick::MB_X1;
      break;
    case SDL_BUTTON_X2:
      click.button_ = interface::MouseClick::MB_X2;
      break;
  }

  click.xcoord_ = event.x;
  click.ycoord_ = event.y;
  click.released_ = event.type == SDL_MOUSEBUTTONUP;
  click.clicks_ = event.clicks;
  return click;
}

void SDLInterface::EventLoop() {
  SDL_Event e;
  while (SDL_PollEvent(&e) != 0) {
    switch (e.type) {
      case SDL_QUIT:
        receiver_->QuitToDesktop();
        break;
      case SDL_KEYUP:
        receiver_->HandleKeyRelease(e.key.keysym);
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if (current_map_.empty()) {
          break;
        }
        if (maps_.find(current_map_) == maps_.end()) {
          break;
        }
        auto& map = maps_.at(current_map_);
        const auto& clicked_id =
            sprites_->ClickedObject(map, e.button.x, e.button.y);
        if (util::objectid::IsNull(clicked_id)) {
          receiver_->HandleMouseEvent(makeMouseClick(e.button));
        } else if (e.type == SDL_MOUSEBUTTONUP) {
          receiver_->SelectObject(clicked_id);
          auto* unit = units::ById(clicked_id);
          if (unit != nullptr) {
            selected_unit_ = unit;
          } else {
            auto* area = geography::ById(clicked_id);
            if (area != nullptr) {
              selected_area_ = area;
            }
          }
        }
        break;
    }
  }

  // TODO: Put this in a separate thread, like a big boy.
  drawMap();
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

Area::Area(const proto::Area& proto, const proto::LatLong& topleft,
           int seconds_per_pixel_high, int seconds_per_pixel_wide) {
  int seconds = seconds_north(topleft) - seconds_north(proto.position());
  ypos_ = seconds / seconds_per_pixel_high;
  seconds = seconds_east(proto.position()) - seconds_east(topleft);
  xpos_ = seconds / seconds_per_pixel_wide;
  area_id_ = proto.area_id();
  draw_location_ = {xpos_ - 5, ypos_ - 5, 10, 10};
}

Map::Map(const proto::Map& proto) : name_(proto.name()) {}

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

  status = sprites_->LoadFonts(base_path, scenario);
  if (!status.ok()) {
    return status;
  }

  for (const proto::Map& map : scenario.maps()) {
    if (maps_.find(map.name()) != maps_.end()) {
      return util::InvalidArgumentError(
          absl::Substitute("Duplicate map name $0", map.name()));
    }

    Map curr(map);
    auto current_path = base_path / map.filename();
    status = sprites_->MapGraphics(current_path, &curr);
    if (!status.ok()) {
      return status;
    }
    int seconds_per_pixel_high =
        seconds_north(map.left_top()) - seconds_north(map.right_bottom());
    seconds_per_pixel_high /= map_rectangle_.h;
    int seconds_per_pixel_wide =
        seconds_east(map.right_bottom()) - seconds_east(map.left_top());
    seconds_per_pixel_wide /= map_rectangle_.w;
    for (const auto& area_proto : map.areas()) {
      area_map_[area_proto.area_id()].second = curr.areas_.size();
      curr.areas_.emplace_back(area_proto, map.left_top(),
                               seconds_per_pixel_high, seconds_per_pixel_wide);
    }
    maps_.emplace(map.name(), std::move(curr));
    current_map_ = map.name();
  }

  status = sprites_->UnitGraphics(base_path, scenario);
  if (!status.ok()) {
    return status;
  }

  return util::OkStatus();
}

}  // namespace graphics
}  // namespace sevenyears
