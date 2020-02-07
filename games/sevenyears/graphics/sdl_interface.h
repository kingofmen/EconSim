#ifndef GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H
#define GAMES_SEVENYEARS_GRAPHICS_SDL_INTERFACE_H

#include <functional>
#include <memory>

#include "interface/base.h"
#include "interface/proto/config.pb.h"
#include "util/status/status.h"
#include "SDL.h"

namespace sevenyears {
namespace graphics {

class SDLInterface : public interface::Base {
public:
  util::Status Initialise(const interface::proto::Config& config) override;
  void Cleanup() override;
  void EventLoop() override;

private:
  struct SDLWindowCleaner {
    void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
  };
  std::unique_ptr<SDL_Window, SDLWindowCleaner> window_;
};

}  // namespace graphics
}  // namespace sevenyears

#endif
