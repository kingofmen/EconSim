#include "games/sevenyears/test_utils.h"

#include "absl/strings/substitute.h"
#include "games/industry/industry.h"
#include "games/setup/setup.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

util::Status
TestState::Initialise(const games::setup::proto::ScenarioFiles& config) {
  auto status = games::setup::LoadScenario(config, &scenario_proto_);
  if (!status.ok()) {
    return status;
  }
  status = games::setup::LoadWorld(config, &world_proto_);
  if (!status.ok()) {
    return status;
  }
  constants_ = games::setup::Constants(scenario_proto_);
  world_ = games::setup::World::FromProto(world_proto_);
  sevenyears::proto::WorldState* world_state = world_proto_.MutableExtension(
      sevenyears::proto::WorldState::sevenyears_state);
  for (const auto& as : world_state->area_states()) {
    state_map_[as.area_id()] = as;
  }
  return util::OkStatus();
}

util::Status
TestState::Initialise(const std::string& location) {
  const std::string kTestDir = std::getenv("TEST_SRCDIR");
  const std::string kWorkdir = std::getenv("TEST_WORKSPACE");
  const std::string kBase =
      absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, location}, "/");

  games::setup::proto::ScenarioFiles config;
  config.add_unit_templates(absl::StrJoin({kBase, kTemplates}, "/"));
  config.set_world_file(absl::StrJoin({kBase, kWorld}, "/"));
  return Initialise(config);
}


const games::setup::World& TestState::World() const {
  return *world_;
}

const games::setup::Constants& TestState::Constants() const {
  return constants_;
}

const proto::AreaState&
TestState::AreaState(const util::proto::ObjectId& area_id) const {
  if (state_map_.find(area_id) == state_map_.end()) {
    Log::Errorf("No state for area %s", util::objectid::DisplayString(area_id));
    static proto::AreaState dummy;
    return dummy;
  }
  return state_map_.at(area_id);
}

const industry::Production&
TestState::ProductionChain(const std::string& name) const {
  static industry::Production dummy;
  return dummy;
}

}  // namespace sevenyears
