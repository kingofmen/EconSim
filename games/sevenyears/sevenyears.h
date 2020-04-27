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
  util::Status InitialiseAI();

  const games::setup::World& World() const { return *game_world_; }
  const sevenyears::proto::AreaState&
  AreaState(const util::proto::ObjectId& area_id) const;

private:
  void moveUnits();

  bool dirtyGraphics_;
  std::unique_ptr<games::setup::World> game_world_;
  games::setup::Constants constants_;
  std::unordered_map<std::string, industry::Production> production_chains_;
  std::unordered_map<util::proto::ObjectId, sevenyears::proto::AreaState>
      area_states_;
};

}  // namespace sevenyears

#endif
