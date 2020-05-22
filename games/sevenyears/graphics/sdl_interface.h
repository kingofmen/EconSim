#ifndef GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H
#define GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H

#include <experimental/filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "games/sevenyears/graphics/proto/graphics.pb.h"
#include "games/sevenyears/graphics/sevenyears_interface.h"
#include "games/interface/proto/config.pb.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.pb.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"
#include "SDL.h"

namespace sevenyears {
namespace graphics {

struct Area {
  Area(const sevenyears::graphics::proto::Area& proto,
       const sevenyears::graphics::proto::LatLong& topleft, int width,
       int height);
  // From top-left of screen.
  int xpos_;
  int ypos_;
  util::proto::ObjectId area_id_;
  std::unordered_map<std::string, int> unit_numbers_;
};

struct Map {
  Map(const sevenyears::graphics::proto::Map& proto);
  std::vector<Area> areas_;
  // Map from template kind to graphic locations.
  std::unordered_map<std::string, std::vector<SDL_Rect>> unit_locations_;
  std::string name_;
};

class SpriteDrawer {
 public:
  virtual void Cleanup() = 0;
  virtual void ClearScreen() = 0;
  virtual void DrawArea(const Area& area) = 0;
  virtual void DrawMap(const Map& map, SDL_Rect* rect) = 0;
  virtual util::Status Init(int width, int height) = 0;
  virtual util::Status
  UnitGraphics(const std::experimental::filesystem::path& base_path,
               const proto::Scenario& scenario) = 0;
  virtual util::Status
  MapGraphics(const std::experimental::filesystem::path& path, Map* map) = 0;
  virtual void Update() = 0;
};

class SDLSpriteDrawer : public SpriteDrawer {
public:
  void Cleanup() override;
  void ClearScreen() override;
  void DrawArea(const Area& area) override;
  void DrawMap(const Map& map, SDL_Rect* rect) override;
  util::Status Init(int width, int height) override;
  util::Status
  UnitGraphics(const std::experimental::filesystem::path& base_path,
               const proto::Scenario& scenario) override;
  util::Status MapGraphics(const std::experimental::filesystem::path& path,
                           Map* map) override;
  void Update() override;

private:
  struct SDLWindowCleaner {
    void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
  };
  struct SDLRendererCleaner {
    void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
  };

  std::unique_ptr<SDL_Window, SDLWindowCleaner> window_;
  std::unique_ptr<SDL_Renderer, SDLRendererCleaner> renderer_;
  std::unordered_map<std::string, SDL_Texture*> unit_types_;
  std::unordered_map<std::string, SDL_Texture*> map_backgrounds_;
};

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
};

}  // namespace graphics
}  // namespace sevenyears

#endif
