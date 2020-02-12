#ifndef GAMES_SEVENYEARS_GRAPHICS_BITMAP_H
#define GAMES_SEVENYEARS_GRAPHICS_BITMAP_H

#include <experimental/filesystem>

#include "util/status/status.h"
#include "SDL.h"

namespace sevenyears {
namespace graphics {
namespace bitmap {


// Loads the file into the pointer.
util::Status LoadForSdl(const std::experimental::filesystem::path file,
                        SDL_Surface*& img);


}  // namespace bitmap
}  // namespace graphics
}  // namespace sevenyears

#endif
