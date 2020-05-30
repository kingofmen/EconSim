#ifndef GAMES_SEVENYEARS_GRAPHICS_SDL_SPRITES_H
#define GAMES_SEVENYEARS_GRAPHICS_SDL_SPRITES_H

#include <experimental/filesystem>
#include <memory>
#include <string>
#include <unordered_map>

#include "games/actions/proto/plan.pb.h"
#include "games/interface/proto/config.pb.h"
#include "games/market/proto/goods.pb.h"
#include "games/sevenyears/graphics/proto/graphics.pb.h"
#include "games/units/unit.h"
#include "util/proto/object_id.pb.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"
#include "SDL.h"
#include "SDL_ttf.h"

struct TextKey {
  std::string str_;
  SDL_Color color_;
};

namespace std {
template <> class hash<TextKey> {
public:
  size_t operator()(const TextKey& key) const {
    static hash<int> hasher;
    static hash<std::string> str_hasher;
    int total = (key.color_.r << 24) + (key.color_.g << 16) +
                (key.color_.b << 8) + key.color_.a;
    return 31*str_hasher(key.str_) + hasher(total);
  }
};

template <> struct equal_to<TextKey> {
  bool operator()(const TextKey& lhs,
                  const TextKey& rhs) const {
    if (lhs.color_.r != rhs.color_.r) {
      return false;
    }
    if (lhs.color_.g != rhs.color_.g) {
      return false;
    }
    if (lhs.color_.b != rhs.color_.b) {
      return false;
    }
    if (lhs.color_.a != rhs.color_.a) {
      return false;
    }
    return lhs.str_ == rhs.str_;
  }
};

} // namespace std


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
  SDL_Rect draw_location_;
};

struct Map {
  Map(const sevenyears::graphics::proto::Map& proto);
  std::vector<Area> areas_;
  std::unordered_map<util::proto::ObjectId, SDL_Rect> unit_locations_;
  std::string name_;
};

struct SDLWindowCleaner {
  void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
};

struct SDLRendererCleaner {
  void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
};

class SpriteDrawer {
 public:
  virtual void Cleanup() = 0;
  virtual void ClearScreen() = 0;
  virtual const util::proto::ObjectId& ClickedObject(const Map& map, int x,
                                                     int y);
  virtual void DrawMap(const Map& map, SDL_Rect* rect) = 0;
  virtual void DrawSelected(const util::proto::ObjectId& object_id,
                            SDL_Rect* unit_rect, SDL_Rect* area_rect);
  virtual util::Status Init(int width, int height) = 0;
  virtual util::Status
  LoadFonts(const std::experimental::filesystem::path& base_path,
            const proto::Scenario& scenario);
  virtual util::Status
  UnitGraphics(const std::experimental::filesystem::path& base_path,
               const proto::Scenario& scenario) = 0;
  virtual util::Status
  MapGraphics(const std::experimental::filesystem::path& path, Map* map) = 0;
  virtual void Update() = 0;
};

// Wrapper for SDL texture.
struct Text {
  SDL_Texture* letters;
  int width;
  int height;
};

class SDLSpriteDrawer : public SpriteDrawer {
public:
  void Cleanup() override;
  void ClearScreen() override;
  const util::proto::ObjectId& ClickedObject(const Map& map, int x,
                                             int y) override;
  void DrawMap(const Map& map, SDL_Rect* rect) override;
  void DrawSelected(const util::proto::ObjectId& object_id, SDL_Rect* unit_rect,
                    SDL_Rect* area_rect) override;
  util::Status Init(int width, int height) override;
  util::Status LoadFonts(const std::experimental::filesystem::path& base_path,
                         const proto::Scenario& scenario) override;
  util::Status
  UnitGraphics(const std::experimental::filesystem::path& base_path,
               const proto::Scenario& scenario) override;
  util::Status MapGraphics(const std::experimental::filesystem::path& path,
                           Map* map) override;
  void Update() override;

private:
  util::Status createText(const std::string& str, const SDL_Color& c);
  Text& getOrCreate(const std::string& str, const SDL_Color& c);
  SDL_Point displayLine(const std::vector<std::string>& texts,
                        const SDL_Color& c, int x, int y);
  SDL_Point displayPlan(const units::Unit& unit, const SDL_Color& c, int x,
                        int y);
  SDL_Point displayResources(const market::proto::Container& resources,
                             const SDL_Color& c, int x, int y);
  SDL_Point displayString(const std::string& str, const SDL_Color& c, int x,
                          int y);
  SDL_Point displayText(Text& text, int x, int y);
  void drawArea(const Area& area);

  std::unique_ptr<SDL_Window, SDLWindowCleaner> window_;
  std::unique_ptr<SDL_Renderer, SDLRendererCleaner> renderer_;
  std::unordered_map<std::string, SDL_Texture*> unit_types_;
  std::unordered_map<std::string, SDL_Texture*> map_backgrounds_;
  std::vector<TTF_Font*> fonts_;
  std::unordered_map<TextKey, Text> display_texts_;
};

// This implementation is not complete, due to my decision to use
// SDL_TTF instead; it is left here in a partial state in case I
// decide to go with OpenGL in the future.
class OpenGLSpriteDrawer : public SpriteDrawer {
public:
  void Cleanup() override;
  void ClearScreen() override;
  void DrawMap(const Map& map, SDL_Rect* rect) override;
  util::Status Init(int width, int height) override;
  util::Status
  UnitGraphics(const std::experimental::filesystem::path& base_path,
               const proto::Scenario& scenario) override;
  util::Status MapGraphics(const std::experimental::filesystem::path& path,
                           Map* map) override;
  void Update() override;

private:
  void drawArea(const Area& area);

  SDL_GLContext gl_context_;
  std::unique_ptr<SDL_Window, SDLWindowCleaner> window_;
  std::unordered_map<std::string, SDL_Texture*> unit_types_;
  std::unordered_map<std::string, SDL_Texture*> map_backgrounds_;
};

}  // namespace graphics
}  // namespace sevenyears

#endif
