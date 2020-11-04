#ifndef GAMES_SEVENYEARS_TEST_UTILS_H
#define GAMES_SEVENYEARS_TEST_UTILS_H

#include "games/industry/industry.h"
#include "games/setup/setup.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "games/sevenyears/proto/testdata.pb.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"

namespace sevenyears {

// Struct to hold golden protos for tests.
struct Golden {
  std::unique_ptr<std::unordered_map<std::string, actions::proto::Plan*>>
      plans_;
  std::unique_ptr<std::unordered_map<
      std::string, sevenyears::testdata::proto::AreaStateList*>>
      area_states_;

  void Plans();
  void AreaStates();
};

void PopulateScenarioFiles(const std::string& location,
                           games::setup::proto::ScenarioFiles* config);

// Loads golden protobufs into the provided object.
util::Status LoadGoldens(const std::string& location, Golden* golds);

// Returns a filename for the provided object ID.
const std::string FileTag(const util::proto::ObjectId& obj_id);

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
  uint64 timestamp() const override { return timestamp_; }

  sevenyears::proto::AreaState*
  mutable_area_state(const util::proto::ObjectId& area_id) override;

private:
  std::unordered_map<util::proto::ObjectId, proto::AreaState> state_map_;
  std::unique_ptr<games::setup::World> world_;
  games::setup::Constants constants_;
  games::setup::proto::GameWorld world_proto_;
  games::setup::proto::Scenario scenario_proto_;
  uint64 timestamp_;
};

}  // namespace sevenyears

#endif
