#ifndef GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H
#define GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "games/sevenyears/graphics/proto/graphics.pb.h"
#include "games/sevenyears/graphics/sevenyears_interface.h"
#include "interface/proto/config.pb.h"
#include "util/status/status.h"
#include "SDL.h"

namespace sevenyears {
namespace graphics {

class SDLInterface : public SevenYearsInterface {
public:
  util::Status Initialise(const interface::proto::Config& config) override;
  void Cleanup() override;
  void EventLoop() override;

  util::Status ScenarioGraphics(
      const sevenyears::graphics::proto::Scenario& scenario) override;

private:
  void clearScreen();
  void drawMap();

  struct SDLWindowCleaner {
    void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
  };
  std::unique_ptr<SDL_Window, SDLWindowCleaner> window_;
  std::unordered_map<std::string, SDL_Surface*> maps_;
  std::string current_map_;
};

}  // namespace graphics
}  // namespace sevenyears

#endif
