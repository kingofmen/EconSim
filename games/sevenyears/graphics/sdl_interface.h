#ifndef GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H
#define GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "games/sevenyears/graphics/proto/graphics.pb.h"
#include "games/sevenyears/graphics/sevenyears_interface.h"
#include "games/interface/proto/config.pb.h"
#include "util/headers/int_types.h"
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
  struct SDLWindowCleaner {
    void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
  };
  struct SDLRendererCleaner {
    void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
  };

  struct Area {
    Area(const sevenyears::graphics::proto::Area& proto,
         const sevenyears::graphics::proto::LatLong& topleft, int width,
         int height);
    // From top-left of screen.
    int xpos_;
    int ypos_;
    uint64 area_id_;
    std::unordered_map<uint64, int> unit_numbers_;
  };

  struct Map {
    Map(const sevenyears::graphics::proto::Map& proto);
    SDL_Texture* background_;
    std::vector<Area> areas_;
    // Map from template type to graphic locations.
    std::unordered_map<uint64, std::vector<SDL_Rect>> unit_locations_;
    std::string name_;
  };

  void clearScreen();
  Area& getAreaById(uint64 area_id);
  void drawArea(const Area& area);
  void drawMap();
  void drawUnit();
  util::Status validate(const sevenyears::graphics::proto::Scenario& scenario);

  std::unique_ptr<SDL_Window, SDLWindowCleaner> window_;
  std::unique_ptr<SDL_Renderer, SDLRendererCleaner> renderer_;
  std::unordered_map<std::string, Map> maps_;
  std::unordered_map<uint64, SDL_Texture*> unit_types_;
  std::unordered_map<uint64, std::pair<std::string, uint64>> area_map_;
  std::string current_map_;
  SDL_Rect map_rectangle_;
};

}  // namespace graphics
}  // namespace sevenyears

#endif
