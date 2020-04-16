#ifndef GAMES_SEVENYEARS_SEVENYEARS_H
#define GAMES_SEVENYEARS_SEVENYEARS_H

#include "games/interface/base.h"
#include "games/setup/proto/setup.pb.h"
#include "games/setup/setup.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/status/status.h"

namespace sevenyears {

// Class for running actual game mechanics.
class SevenYears {
public:
  SevenYears() : dirtyGraphics_(true) {}
  ~SevenYears() {}

  util::Status LoadScenario(const games::setup::proto::ScenarioFiles& setup);
  void NewTurn();
  void UpdateGraphicsInfo(interface::Base* gfx);

private:
  bool dirtyGraphics_;
  std::unique_ptr<games::setup::World> game_world_;
  games::setup::Constants constants_;
  std::unordered_map<std::string, int> chain_indices_;
  std::unordered_map<util::proto::ObjectId, sevenyears::proto::AreaState>
      area_states_;
};

util::Status InitialiseAI();

}  // namespace sevenyears

#endif
