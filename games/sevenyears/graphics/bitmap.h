#ifndef GAMES_SEVENYEARS_GRAPHICS_BITMAP_H
#define GAMES_SEVENYEARS_GRAPHICS_BITMAP_H

// TODO: Upgrade to C++17
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

#include "util/status/status.h"
#include "SDL.h"

namespace sevenyears {
namespace graphics {
namespace bitmap {


// Loads the file into the pointer.
util::Status LoadForSdl(const std::experimental::filesystem::path file,
                        SDL_Surface*& img);

// Creates a texture from the file and loads it into the texture pointer.
util::Status MakeTexture(const std::experimental::filesystem::path& file,
                         SDL_Renderer* renderer, SDL_Texture*& tex);

}  // namespace bitmap
}  // namespace graphics
}  // namespace sevenyears

#endif
