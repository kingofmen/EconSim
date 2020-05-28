#include "games/sevenyears/graphics/sdl_sprites.h"

#include "absl/strings/substitute.h"
#include "games/geography/geography.h"
#include "games/geography/connection.h"
#include "games/interface/proto/config.pb.h"
#include "games/market/proto/goods.pb.h"
#include "games/sevenyears/graphics/bitmap.h"
#include "games/units/unit.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_opengl.h"

namespace sevenyears {
namespace graphics {
namespace {

constexpr char kUnknownString[] = "UNKNOWN_STRING_WANTED";

const SDL_Color kWhite = {255, 255, 255, 0};
const SDL_Color kGold = {198, 165, 48, 0};

bool isWithin(const SDL_Rect& rect, int x, int y) {
  if (x < rect.x) {
    return false;
  }
  if (x > rect.x + rect.w) {
    return false;
  }
  if (y < rect.y) {
    return false;
  }
  if (y > rect.y + rect.h) {
    return false;
  }
  return true;
}

} // namespace

util::Status
SpriteDrawer::LoadFonts(const std::experimental::filesystem::path& base_path,
                        const proto::Scenario& scenario) {
  return util::NotImplementedError(
      "LoadFonts not implemented in this sprite class.");
}

void SpriteDrawer::DrawSelected(const util::proto::ObjectId& object_id,
                                SDL_Rect* unit_rect, SDL_Rect* area_rect) {
  // Do nothing.
}

const util::proto::ObjectId& SpriteDrawer::ClickedObject(const Map& map, int x,
                                                         int y) {
  return util::objectid::kNullId;
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

const util::proto::ObjectId& SDLSpriteDrawer::ClickedObject(const Map& map,
                                                            int x, int y) {
  for (const auto& unit : map.unit_locations_) {
    if (isWithin(unit.second, x, y)) {
      return unit.first;
    }
  }

  for (const auto& area : map.areas_) {
    if (isWithin(area.draw_location_, x, y)) {
      return area.area_id_;
    }
  }

  return util::objectid::kNullId;
}

void SDLSpriteDrawer::drawArea(const Area& area) {
  SDL_SetRenderDrawColor(renderer_.get(), 0xFF, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer_.get(), &area.draw_location_);
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

util::Status SDLSpriteDrawer::createText(const std::string& str, const SDL_Color& c) {
  TextKey key = {str, c};
  if (display_texts_.find(key) != display_texts_.end()) {
    return util::OkStatus();
  }
  SDL_Surface* nameSurface =
      TTF_RenderText_Solid(fonts_[0], str.c_str(), c);
  if (nameSurface == NULL) {
    return util::InvalidArgumentError(
        absl::Substitute("Could not create surface for $0", str));
  }
  SDL_Texture* nameTexture =
      SDL_CreateTextureFromSurface(renderer_.get(), nameSurface);
  if (nameTexture == NULL) {
    return util::InvalidArgumentError(
        absl::Substitute("Could not create texture for $0", str));
  }
  display_texts_[key] = {nameTexture, nameSurface->w, nameSurface->h};
  SDL_FreeSurface(nameSurface);
  return util::OkStatus();
}

SDL_Point SDLSpriteDrawer::displayText(Text& text, int x, int y) {
  SDL_Rect target = {x, y, text.width, text.height};
  SDL_RenderCopy(renderer_.get(), text.letters, NULL, &target);
  return {x + text.width, y + text.height};
}

SDL_Point SDLSpriteDrawer::displayString(const std::string& str,
                                         const SDL_Color& c, int x, int y) {
  auto& text = getOrCreate(str, c);
  return displayText(text, x, y);
}

SDL_Point
SDLSpriteDrawer::displayResources(const market::proto::Container& resources,
                                  const SDL_Color& c, int x, int y) {
  SDL_Point current = {x, y};
  for (const auto& q : resources.quantities()) {
    if (q.second < 1) {
      continue;
    }
    auto nameText = getOrCreate(q.first, c);
    auto next = displayText(nameText, current.x, current.y);
    auto amountText = getOrCreate(micro::DisplayString(q.second, 2), c);
    next = displayText(amountText, next.x + 3, current.y);
    current.y = next.y + 3;
  }
  return current;
}

std::vector<std::string> stepToStrings(const actions::proto::Step& step,
                                       util::proto::ObjectId* area_id) {
  std::vector<std::string> ret;
  switch (step.trigger_case()) {
  case actions::proto::Step::kKey:
    return {"Do ", step.key(), " in ", util::objectid::DisplayString(*area_id)};
  case actions::proto::Step::kAction: {
    switch (step.action()) {
    case actions::proto::AA_MOVE: {
      ret.push_back("Move ");
      ret.push_back(util::objectid::DisplayString(*area_id));
      ret.push_back(" -> ");
      std::string nextLocation = "unknown";
      if (step.has_connection_id()) {
        auto* conn = geography::Connection::ById(step.connection_id());
        if (conn != nullptr) {
          *area_id = conn->OtherSide(*area_id);
          nextLocation = util::objectid::DisplayString(*area_id);
        }
      }
      ret.push_back(nextLocation);
      return ret;
    }
    default:
      break;
    }
    break;
  }
  default:
    break;
  }
  return ret;
}

SDL_Point SDLSpriteDrawer::displayPlan(const units::Unit& unit,
                                       const SDL_Color& c, int x, int y) {
  const actions::proto::Plan& plan = unit.plan();
  if (plan.steps().empty()) {
    return displayString("No current plan", c, x, y);
  }
  const auto& location = unit.location();
  auto area_id = location.a_area_id();
  SDL_Point current = {x, y};
  for (const auto& step : plan.steps()) {
    SDL_Point next = current;
    auto texts = stepToStrings(step, &area_id);
    for (const auto& text : texts) {
      auto& texture = getOrCreate(text, c);
      next = displayText(texture, next.x, current.y);
    }
    current.y = next.y + 3;
  }
  return current;
}

void SDLSpriteDrawer::DrawMap(const Map& map, SDL_Rect* rect) {
  SDL_RenderCopy(renderer_.get(), map_backgrounds_[map.name_], NULL, rect);
  for (const auto& unit : map.unit_locations_) {
    if (unit_types_.find(unit.first.kind()) == unit_types_.end()) {
      continue;
    }
    auto* icon = unit_types_[unit.first.kind()];
    SDL_RenderCopy(renderer_.get(), icon, NULL, &unit.second);
  }
  for (const Area& area : map.areas_) {
    drawArea(area);
  }
  displayString(map.name_, kWhite, 5, 5);
}

Text& SDLSpriteDrawer::getOrCreate(const std::string& str, const SDL_Color& c) {
  TextKey key = {str, c};
  if (display_texts_.find(key) != display_texts_.end()) {
    return display_texts_.at(key);
  }
  auto status = createText(str, c);
  if (status.ok()) {
    return display_texts_.at(key);
  }
  display_texts_[key] = display_texts_[{kUnknownString, c}];
  return display_texts_.at(key);
}

void SDLSpriteDrawer::DrawSelected(const util::proto::ObjectId& object_id,
                                   SDL_Rect* unit_rect, SDL_Rect* area_rect) {
  SDL_SetRenderDrawColor(renderer_.get(), 0x00, 0x00, 0x00, 0xFF);
  SDL_RenderFillRect(renderer_.get(), unit_rect);
  SDL_RenderFillRect(renderer_.get(), area_rect);
  auto& nameText = getOrCreate(util::objectid::DisplayString(object_id), kGold);
  const units::Unit* unit = units::ById(object_id);
  if (unit != nullptr) {
    auto next = displayText(nameText, unit_rect->x + 5, unit_rect->y + 5);
    next = displayResources(unit->resources(), kGold, unit_rect->x + 5,
                            next.y + 3);
    next = displayPlan(*unit, kGold, unit_rect->x + 5, next.y + 3);
  }
  geography::Area* area = geography::ById(object_id);
  if (area != nullptr) {
    displayText(nameText, area_rect->x + 5, area_rect->y + 5);
  }
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
  auto status = createText(kUnknownString, kWhite);
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

  status = createText(map->name_, kWhite);
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

void OpenGLSpriteDrawer::drawArea(const Area& area) {
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

