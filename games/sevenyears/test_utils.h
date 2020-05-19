#ifndef GAMES_SEVENYEARS_TEST_UTILS_H
#define GAMES_SEVENYEARS_TEST_UTILS_H

#include "games/industry/industry.h"
#include "games/setup/setup.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

const std::string kTestDataLocation = "games/sevenyears/test_data";
const std::string kTemplates = "templates.pb.txt";
const std::string kWorld = "world.pb.txt";

class TestState : public SevenYearsState {
public:
  TestState() {}

  util::Status Initialise(const games::setup::proto::ScenarioFiles& config);
  util::Status Initialise(const std::string& location);

  const games::setup::World& World() const override;
  const games::setup::Constants& Constants() const override;
  const proto::AreaState&
  AreaState(const util::proto::ObjectId& area_id) const override;
  const industry::Production&
  ProductionChain(const std::string& name) const override;

private:
  std::unordered_map<util::proto::ObjectId, proto::AreaState> state_map_;
  std::unique_ptr<games::setup::World> world_;
  games::setup::Constants constants_;
  games::setup::proto::GameWorld world_proto_;
  games::setup::proto::Scenario scenario_proto_;
};

}  // namespace sevenyears

#endif
