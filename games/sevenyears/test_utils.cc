#include "games/sevenyears/test_utils.h"

#include <experimental/filesystem>

#include "absl/strings/substitute.h"
#include "games/industry/industry.h"
#include "games/setup/setup.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

const std::string kTestDataLocation = "games/sevenyears/test_data";
const std::string kTemplates = "templates.pb.txt";
const std::string kWorld = "world.pb.txt";
const std::string kGoods = "trade_goods.pb.txt";
const std::string kChains = "chains.pb.txt";
//const std::string kConsumption = "consumption.pb.txt";

void PopulateScenarioFiles(const std::string& location,
                           games::setup::proto::ScenarioFiles* config) {
  const std::string kTestDir = std::getenv("TEST_SRCDIR");
  const std::string kWorkdir = std::getenv("TEST_WORKSPACE");
  const std::string kBase =
      absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, location}, "/");

  auto filename = absl::StrJoin({kBase, kTemplates}, "/");
  if (std::experimental::filesystem::exists(filename)) {
    config->add_unit_templates(filename);
  }
  filename = absl::StrJoin({kBase, kGoods}, "/");
  if (std::experimental::filesystem::exists(filename)) {
    config->add_trade_goods(absl::StrJoin({kBase, kGoods}, "/"));
  }
  filename = absl::StrJoin({kBase, kChains}, "/");
  if (std::experimental::filesystem::exists(filename)) {
    config->add_production_chains(absl::StrJoin({kBase, kChains}, "/"));
  }
  config->set_world_file(absl::StrJoin({kBase, kWorld}, "/"));
  config->set_name(kBase);
  config->set_description(absl::Substitute("Unit test '$0'", location));
}

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
  games::setup::proto::ScenarioFiles config;
  PopulateScenarioFiles(location, &config);
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
