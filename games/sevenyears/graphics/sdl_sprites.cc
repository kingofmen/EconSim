#include "games/sevenyears/graphics/sdl_sprites.h"

#include "absl/strings/substitute.h"
#include "games/interface/proto/config.pb.h"
#include "games/sevenyears/graphics/bitmap.h"
#include "util/proto/object_id.pb.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"

#include "SDL.h"
#include "SDL_opengl.h"

namespace sevenyears {
namespace graphics {

util::Status SDLSpriteDrawer::Init(int width, int height) {
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
  return util::OkStatus();
}

void SDLSpriteDrawer::Cleanup() {
  for (auto& mb : map_backgrounds_) {
    SDL_DestroyTexture(mb.second);
  }
  for (auto& ut : unit_types_) {
    SDL_DestroyTexture(ut.second);
  }
  window_.reset(nullptr); // Also calls deleter.
  renderer_.reset(nullptr);
}

void SDLSpriteDrawer::ClearScreen() {
  SDL_SetRenderDrawColor(renderer_.get(), 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderClear(renderer_.get());
}

void SDLSpriteDrawer::DrawArea(const Area& area) {
  SDL_Rect fillRect = {area.xpos_ - 5, area.ypos_ - 5, 10, 10};
  SDL_SetRenderDrawColor(renderer_.get(), 0xFF, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer_.get(), &fillRect);
  int numTypes = area.unit_numbers_.size();
  static const int kIncrement = 24;
  static const int kWidth = 16;
  int start = area.xpos_ - kIncrement * (numTypes / 2) - kWidth / 2;
  start += (kIncrement / 2) * (1 - numTypes % 2);
  int idx = 0;
  SDL_Rect loc;
  loc.w = kWidth;
  loc.h = kWidth;
  for (const auto& units : area.unit_numbers_) {
    SDL_Texture* tex = unit_types_[units.first];
    int number = units.second;
    loc.x = start + kIncrement * idx++;
    loc.y = area.ypos_ - (kIncrement/2 + kWidth);
    for (int i = 0; i < number; ++i) {
      SDL_RenderCopy(renderer_.get(), tex, NULL, &loc);
      loc.x += 3;
      loc.y -= 3;
    }
  }
}

void SDLSpriteDrawer::DrawMap(const Map& map, SDL_Rect* rect) {
  SDL_RenderCopy(renderer_.get(), map_backgrounds_[map.name_], NULL, rect);
  for (const auto& unit : map.unit_locations_) {
    if (unit_types_.find(unit.first) == unit_types_.end()) {
      continue;
    }
    auto* icon = unit_types_[unit.first];
    for (const auto& loc : unit.second) {
      SDL_RenderCopy(renderer_.get(), icon, NULL, &loc);
    }
  }
}

util::Status SDLSpriteDrawer::UnitGraphics(
    const std::experimental::filesystem::path& base_path,
    const proto::Scenario& scenario) {
  for (const auto& ut : scenario.unit_types()) {
    if (unit_types_.find(ut.template_kind()) != unit_types_.end()) {
      return util::InvalidArgumentError(
          absl::Substitute("Duplicate unit graphics for $0: $1",
                           ut.display_name(), ut.DebugString()));
    }
    auto current_path = base_path / ut.filename();
    SDL_Texture* tex = NULL;
    auto status = bitmap::MakeTexture(current_path, renderer_.get(), tex);
    if (!status.ok()) {
      return status;
    }
    unit_types_[ut.template_kind()] = tex;
  }
  return util::OkStatus();
}

util::Status
SDLSpriteDrawer::MapGraphics(const std::experimental::filesystem::path& path,
                             Map* map) {
  SDL_Texture* bkg = NULL;
  auto status = bitmap::MakeTexture(path, renderer_.get(), bkg);
  if (!status.ok()) {
    return status;
  }
  map_backgrounds_[map->name_] = bkg;
  return util::OkStatus();
}

void SDLSpriteDrawer::Update() {
  SDL_UpdateWindowSurface(window_.get());
  SDL_RenderPresent(renderer_.get());
}

}  // namespace graphics
}  // namespace sevenyears

