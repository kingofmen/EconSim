#include "games/sevenyears/graphics/sdl_interface.h"

#include "absl/strings/substitute.h"
#include "util/status/status.h"
#include "SDL.h"


namespace sevenyears {
namespace graphics {

util::Status SDLInterface::Initialise(const interface::proto::Config& config) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not initialise SDL: $0", SDL_GetError()));
  }

  window_.reset(SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, 640, 480,
                                 SDL_WINDOW_SHOWN));
  if (!window_) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not create window: $0", SDL_GetError()));
  }

  SDL_Surface* screenSurface = SDL_GetWindowSurface(window_.get());
  SDL_FillRect(screenSurface, NULL,
               SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
  SDL_UpdateWindowSurface(window_.get());
  return util::OkStatus();
}

void SDLInterface::Cleanup() {
  window_.reset(nullptr); // Also calls deleter.
  SDL_Quit();
}

}  // namespace graphics
}  // namespace sevenyears
