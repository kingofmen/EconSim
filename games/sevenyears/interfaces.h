#ifndef GAMES_SEVENYEARS_INTERFACES_H
#define GAMES_SEVENYEARS_INTERFACES_H

#include "games/industry/industry.h"
#include "games/setup/setup.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/proto/object_id.pb.h"

namespace sevenyears {

// Abstract interface for moving game-state information around.
class SevenYearsState : public games::setup::WorldStateInterface {
public:
  // Const methods.
  virtual const games::setup::World& World() const override = 0;
  virtual const games::setup::Constants& Constants() const override = 0;
  virtual const sevenyears::proto::AreaState&
  AreaState(const util::proto::ObjectId& area_id) const = 0;
  virtual const industry::Production&
  ProductionChain(const std::string& name) const = 0;
  virtual uint64 timestamp() const = 0;

  // Non-const methods.
  virtual sevenyears::proto::AreaState*
  mutable_area_state(const util::proto::ObjectId& area_id) = 0;
};

// Default implementation.
class SevenYearsStateImpl : public SevenYearsState {
public:
  // Time-related methods.
  uint64 timestamp() const override { return timestamp_; }
  void incrementTime() { timestamp_++; }
  void setTime(uint64 t) { timestamp_ = t; }

  // World-state accessors.
  const games::setup::World& World() const override { return *game_world_; }
  const games::setup::Constants& Constants() const override {
    return constants_;
  }

protected:
  std::unique_ptr<games::setup::World> game_world_;
  games::setup::Constants constants_;

private:
  uint64 timestamp_;
};

}  // namespace sevenyears



#endif
