#ifndef GAMES_SEVENYEARS_GRAPHICS_SEVENYEARS_INTERFACE_H
#define GAMES_SEVENYEARS_GRAPHICS_SEVENYEARS_INTERFACE_H

#include "games/sevenyears/graphics/proto/graphics.pb.h"
#include "games/interface/base.h"
#include "games/interface/proto/config.pb.h"
#include "util/status/status.h"

namespace sevenyears {
namespace graphics {

class SevenYearsInterface : public interface::Base {
public:
  virtual util::Status
  Initialise(const games::interface::proto::Config& config) = 0;
  virtual void Cleanup() = 0;
  virtual void EventLoop() = 0;

  virtual util::Status
  ScenarioGraphics(const sevenyears::graphics::proto::Scenario& scenario) = 0;

private:
};

}  // namespace graphics
}  // namespace sevenyears

#endif
