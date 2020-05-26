#ifndef GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H
#define GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H

#include <memory>
#include <string>
#include <unordered_map>

#include "games/geography/geography.h"
#include "games/interface/proto/config.pb.h"
#include "games/sevenyears/graphics/proto/graphics.pb.h"
#include "games/sevenyears/graphics/sdl_sprites.h"
#include "games/sevenyears/graphics/sevenyears_interface.h"
#include "games/units/unit.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.pb.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"
#include "SDL.h"

namespace sevenyears {
namespace graphics {

class SDLInterface : public SevenYearsInterface {
public:
  util::Status
  Initialise(const games::interface::proto::Config& config) override;
  void Cleanup() override;
  void EventLoop() override;

  util::Status ScenarioGraphics(
      const sevenyears::graphics::proto::Scenario& scenario) override;
  void DisplayUnits(const std::vector<util::proto::ObjectId>& ids) override;

private:
  Area& getAreaById(const util::proto::ObjectId& area_id);
  void drawMap();
  util::Status validate(const sevenyears::graphics::proto::Scenario& scenario);

  SpriteDrawer* sprites_;
  std::unordered_map<std::string, Map> maps_;
  std::unordered_map<util::proto::ObjectId, std::pair<std::string, uint64>>
      area_map_;
  std::string current_map_;
  SDL_Rect map_rectangle_;
  SDL_Rect unit_status_rectangle_;
  SDL_Rect area_status_rectangle_;
  util::proto::ObjectId selected_;
};

}  // namespace graphics
}  // namespace sevenyears

#endif
