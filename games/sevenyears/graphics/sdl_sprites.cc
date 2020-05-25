#include "games/sevenyears/graphics/sdl_sprites.h"

#include "absl/strings/substitute.h"
#include "games/interface/proto/config.pb.h"
#include "games/sevenyears/graphics/bitmap.h"
#include "util/proto/object_id.pb.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_opengl.h"

namespace sevenyears {
namespace graphics {

constexpr char kUnknownString[] = "UNKNOWN_STRING_WANTED";

util::Status
SpriteDrawer::LoadFonts(const std::experimental::filesystem::path& base_path,
                        const proto::Scenario& scenario) {
  return util::NotImplementedError(
      "LoadFonts not implemented in this sprite class.");
}

util::Status SDLSpriteDrawer::Init(int width, int height) {
  window_.reset(SDL_CreateWindow("Seven Years", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, width, height,
                                 SDL_WINDOW_SHOWN));
  if (!window_) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not create window: $0", SDL_GetError()));
  }
  renderer_.reset(
      SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED));
  if (!renderer_) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not create renderer: $0", SDL_GetError()));
  }

  return util::OkStatus();
}

void SDLSpriteDrawer::Cleanup() {
  for (TTF_Font* font : fonts_) {
    TTF_CloseFont(font);
  }
  for (auto& mb : map_backgrounds_) {
    SDL_DestroyTexture(mb.second);
  }
  for (auto& ut : unit_types_) {
    SDL_DestroyTexture(ut.second);
  }
  window_.reset(nullptr); // Also calls deleter.
  renderer_.reset(nullptr);
}

void SDLSpriteDrawer::ClearScreen() {
  SDL_SetRenderDrawColor(renderer_.get(), 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderClear(renderer_.get());
}

void SDLSpriteDrawer::DrawArea(const Area& area) {
  SDL_Rect fillRect = {area.xpos_ - 5, area.ypos_ - 5, 10, 10};
  SDL_SetRenderDrawColor(renderer_.get(), 0xFF, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer_.get(), &fillRect);
  int numTypes = area.unit_numbers_.size();
  static const int kIncrement = 24;
  static const int kWidth = 16;
  int start = area.xpos_ - kIncrement * (numTypes / 2) - kWidth / 2;
  start += (kIncrement / 2) * (1 - numTypes % 2);
  int idx = 0;
  SDL_Rect loc;
  loc.w = kWidth;
  loc.h = kWidth;
  for (const auto& units : area.unit_numbers_) {
    SDL_Texture* tex = unit_types_[units.first];
    int number = units.second;
    loc.x = start + kIncrement * idx++;
    loc.y = area.ypos_ - (kIncrement/2 + kWidth);
    for (int i = 0; i < number; ++i) {
      SDL_RenderCopy(renderer_.get(), tex, NULL, &loc);
      loc.x += 3;
      loc.y -= 3;
    }
  }
}

util::Status SDLSpriteDrawer::createText(const std::string& str) {
  static SDL_Color nameColor = {0, 0, 0};
  SDL_Surface* nameSurface =
      TTF_RenderText_Solid(fonts_[0], str.c_str(), nameColor);
  if (nameSurface == NULL) {
    return util::InvalidArgumentError(
        absl::Substitute("Could not create surface for $0", str));
  }
  SDL_Texture* nameTexture = SDL_CreateTextureFromSurface(renderer_.get(), nameSurface);
  if (nameTexture == NULL) {
    return util::InvalidArgumentError(
        absl::Substitute("Could not create texture for $0", str));
  }
  display_texts_[str] = {nameTexture, nameSurface->w, nameSurface->h};
  SDL_FreeSurface(nameSurface);
  return util::OkStatus();
}

void SDLSpriteDrawer::displayText(const std::string& str) {
  if (display_texts_.find(str) == display_texts_.end()) {
    if (display_texts_.find(kUnknownString) == display_texts_.end()) {
      // Something went very wrong here.
      return;
    }
    displayText(kUnknownString);
  }
  auto& text = display_texts_.at(str);
  SDL_Rect target = {5, 5, text.width, text.height};
  SDL_RenderCopy(renderer_.get(), text.letters, NULL, &target);  
}

void SDLSpriteDrawer::DrawMap(const Map& map, SDL_Rect* rect) {
  SDL_RenderCopy(renderer_.get(), map_backgrounds_[map.name_], NULL, rect);
  for (const auto& unit : map.unit_locations_) {
    if (unit_types_.find(unit.first) == unit_types_.end()) {
      continue;
    }
    auto* icon = unit_types_[unit.first];
    for (const auto& loc : unit.second) {
      SDL_RenderCopy(renderer_.get(), icon, NULL, &loc);
    }
  }
  displayText(map.name_);
}

util::Status
SDLSpriteDrawer::LoadFonts(const std::experimental::filesystem::path& base_path,
                           const proto::Scenario& scenario) {
  for (const auto& fontfile : scenario.fonts()) {
    auto fullpath = base_path / fontfile;
    if (!std::experimental::filesystem::exists(fullpath)) {
      return util::NotFoundError(
          absl::Substitute("Could not find font path $0", fullpath.string()));
    }

    TTF_Font* font = TTF_OpenFont(fullpath.string().c_str(), 14);
    if (font == NULL) {
      return util::InvalidArgumentError(absl::Substitute(
          "Could not open font $0: $1", fontfile, TTF_GetError()));
    }
    fonts_.push_back(font);
  }
  auto status = createText(kUnknownString);
  if (!status.ok()) {
    return status;
  }

  return util::OkStatus();
}

util::Status SDLSpriteDrawer::UnitGraphics(
    const std::experimental::filesystem::path& base_path,
    const proto::Scenario& scenario) {
  for (const auto& ut : scenario.unit_types()) {
    if (unit_types_.find(ut.template_kind()) != unit_types_.end()) {
      return util::InvalidArgumentError(
          absl::Substitute("Duplicate unit graphics for $0: $1",
                           ut.display_name(), ut.DebugString()));
    }
    auto current_path = base_path / ut.filename();
    SDL_Texture* tex = NULL;
    auto status = bitmap::MakeTexture(current_path, renderer_.get(), tex);
    if (!status.ok()) {
      return status;
    }
    unit_types_[ut.template_kind()] = tex;
  }
  return util::OkStatus();
}

util::Status
SDLSpriteDrawer::MapGraphics(const std::experimental::filesystem::path& path,
                             Map* map) {
  SDL_Texture* bkg = NULL;
  auto status = bitmap::MakeTexture(path, renderer_.get(), bkg);
  if (!status.ok()) {
    return status;
  }
  map_backgrounds_[map->name_] = bkg;

  status = createText(map->name_);
  if (!status.ok()) {
    return status;
  }
  
  return util::OkStatus();
}

void SDLSpriteDrawer::Update() {
  SDL_UpdateWindowSurface(window_.get());
  SDL_RenderPresent(renderer_.get());
}

util::Status OpenGLSpriteDrawer::Init(int width, int height) {
  // Use OpenGL 2.1 for the time being.
  // TODO: Eh, I should probably figure out that rendering pipeline and whatnot.
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

  window_.reset(SDL_CreateWindow("Seven Years", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, width, height,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN));
  gl_context_ = SDL_GL_CreateContext(window_.get());
  if (!gl_context_) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not create GL context: $0", SDL_GetError()));
  }

  if (SDL_GL_SetSwapInterval(1) < 0) {
    return util::FailedPreconditionError(
        absl::Substitute("Could not set swap interval: $0", SDL_GetError()));
  }
  glEnable(GL_TEXTURE_2D);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho( 0.0, width, height, 0.0, 1.0, -1.0 );
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    return util::FailedPreconditionError(
        absl::Substitute("Error initialising OpenGL projection: $0", error));
  }

  // Initialise Modelview Matrix.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  error = glGetError();
  glClearColor( 1.f, 1.f, 1.f, 1.f );
  if (error != GL_NO_ERROR) {
    return util::FailedPreconditionError(
        absl::Substitute("Error initialising OpenGL modelview: $0", error));
  }
  return util::OkStatus();
}

void OpenGLSpriteDrawer::Cleanup() {
  for (auto& mb : map_backgrounds_) {
    SDL_DestroyTexture(mb.second);
  }
  for (auto& ut : unit_types_) {
    SDL_DestroyTexture(ut.second);
  }
  window_.reset(nullptr); // Also calls deleter.
}

void OpenGLSpriteDrawer::ClearScreen() {
  glClear( GL_COLOR_BUFFER_BIT );
}

void OpenGLSpriteDrawer::DrawArea(const Area& area) {
  /*
  SDL_Rect fillRect = {area.xpos_ - 5, area.ypos_ - 5, 10, 10};
  SDL_SetRenderDrawColor(renderer_.get(), 0xFF, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer_.get(), &fillRect);
  int numTypes = area.unit_numbers_.size();
  static const int kIncrement = 24;
  static const int kWidth = 16;
  int start = area.xpos_ - kIncrement * (numTypes / 2) - kWidth / 2;
  start += (kIncrement / 2) * (1 - numTypes % 2);
  int idx = 0;
  SDL_Rect loc;
  loc.w = kWidth;
  loc.h = kWidth;
  for (const auto& units : area.unit_numbers_) {
    SDL_Texture* tex = unit_types_[units.first];
    int number = units.second;
    loc.x = start + kIncrement * idx++;
    loc.y = area.ypos_ - (kIncrement/2 + kWidth);
    for (int i = 0; i < number; ++i) {
      SDL_RenderCopy(renderer_.get(), tex, NULL, &loc);
      loc.x += 3;
      loc.y -= 3;
    }
  }
  */
}

void OpenGLSpriteDrawer::DrawMap(const Map& map, SDL_Rect* rect) {
  /*
  SDL_RenderCopy(renderer_.get(), map_backgrounds_[map.name_], NULL, rect);
  for (const auto& unit : map.unit_locations_) {
    if (unit_types_.find(unit.first) == unit_types_.end()) {
      continue;
    }
    auto* icon = unit_types_[unit.first];
    for (const auto& loc : unit.second) {
      SDL_RenderCopy(renderer_.get(), icon, NULL, &loc);
    }
  }
  */
}

util::Status OpenGLSpriteDrawer::UnitGraphics(
    const std::experimental::filesystem::path& base_path,
    const proto::Scenario& scenario) {
  /*
  for (const auto& ut : scenario.unit_types()) {
    if (unit_types_.find(ut.template_kind()) != unit_types_.end()) {
      return util::InvalidArgumentError(
          absl::Substitute("Duplicate unit graphics for $0: $1",
                           ut.display_name(), ut.DebugString()));
    }
    auto current_path = base_path / ut.filename();
    SDL_Texture* tex = NULL;
    auto status = bitmap::MakeTexture(current_path, renderer_.get(), tex);
    if (!status.ok()) {
      return status;
    }
    unit_types_[ut.template_kind()] = tex;
  }
  */
  return util::OkStatus();
}

util::Status
OpenGLSpriteDrawer::MapGraphics(const std::experimental::filesystem::path& path,
                             Map* map) {
  /*
  SDL_Texture* bkg = NULL;
  auto status = bitmap::MakeTexture(path, renderer_.get(), bkg);
  if (!status.ok()) {
    return status;
  }
  map_backgrounds_[map->name_] = bkg;
  */
  return util::OkStatus();
}

void OpenGLSpriteDrawer::Update() {
  SDL_GL_SwapWindow(window_.get());
}

}  // namespace graphics
}  // namespace sevenyears

