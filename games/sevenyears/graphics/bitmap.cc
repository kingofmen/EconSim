#include "games/sevenyears/graphics/bitmap.h"

#include <experimental/filesystem>

#include "absl/strings/substitute.h"
#include "util/status/status.h"

namespace sevenyears {
namespace graphics {
namespace bitmap {


util::Status LoadForSdl(const std::experimental::filesystem::path file,
                        SDL_Surface*& img) {
  if (!std::experimental::filesystem::exists(file)) {
    return util::NotFoundError(
        absl::Substitute("Could not find file $0", file.string()));
  }

  if (img != nullptr) {
    return util::InvalidArgumentError(
        absl::Substitute("$0 would overwrite image pointer", file.string()));
  }

  img = SDL_LoadBMP(file.string().c_str());
  if (img == nullptr) {
    return util::NotFoundError(absl::Substitute("Error loading $0: $1",
                                                file.string(), SDL_GetError()));
  }
  return util::OkStatus();
}

util::Status MakeTexture(const std::experimental::filesystem::path& file,
                         SDL_Renderer* renderer, SDL_Texture*& tex) {
  SDL_Surface* surface = NULL;
  auto status = LoadForSdl(file, surface);
  if (!status.ok()) {
    return status;
  }
  tex = SDL_CreateTextureFromSurface(renderer, surface);
  if (!tex) {
    return util::InvalidArgumentError(absl::Substitute(
        "Could not convert $0 to texture: $1", file.string(), SDL_GetError()));
  }
  SDL_FreeSurface(surface);
  return util::OkStatus();
}

}  // namespace bitmap
}  // namespace graphics
}  // namespace sevenyears
