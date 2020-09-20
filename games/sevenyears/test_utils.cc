#include "games/sevenyears/test_utils.h"

#include <experimental/filesystem>

#include "absl/strings/substitute.h"
#include "games/industry/industry.h"
#include "games/setup/setup.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/logging/logging.h"
#include "util/proto/file.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

const std::string kTestDataLocation = "games/sevenyears/test_data";
const std::string kTemplates = "templates.pb.txt";
const std::string kWorld = "world.pb.txt";
const std::string kGoods = "trade_goods.pb.txt";
const std::string kChains = "chains.pb.txt";
const std::string kGoldens = "goldens";
const std::string kPlans = "plans";
const std::string kStates = "area_states";
//const std::string kConsumption = "consumption.pb.txt";

namespace {

const std::string baseDir(const std::string& location) {
  const std::string kTestDir = std::getenv("TEST_SRCDIR");
  const std::string kWorkdir = std::getenv("TEST_WORKSPACE");
  return absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, location}, "/");
}

util::Status
loadGoldenPlans(const std::string& base,
                std::unordered_map<std::string, actions::proto::Plan*>* plans) {
  if (plans == nullptr) {
    return util::OkStatus();
  }

  auto dir = absl::StrJoin({base, kPlans}, "/");
  if (!std::experimental::filesystem::exists(dir)) {
    return util::NotFoundError(absl::Substitute("Could not find $0", dir));
  }

  auto& planRef = *plans;
  for (auto& entry : std::experimental::filesystem::directory_iterator(dir)) {
    auto* plan = new actions::proto::Plan();
    auto path = entry.path();
    auto status = util::proto::ParseProtoFile(path.string(), plan);
    if (!status.ok()) {
      return status;
    }
    planRef[path.filename().string()] = plan;
  }
  
  return util::OkStatus();
}

util::Status loadGoldenStates(
    const std::string& base,
    std::unordered_map<std::string, sevenyears::proto::AreaState*>* states) {
  if (states == nullptr) {
    return util::OkStatus();
  }

  auto dir = absl::StrJoin({base, kStates}, "/");
  if (!std::experimental::filesystem::exists(dir)) {
    return util::NotFoundError(absl::Substitute("Could not find $0", dir));
  }

  auto& stateRef = *states;
  for (auto& entry : std::experimental::filesystem::directory_iterator(dir)) {
    auto* state = new sevenyears::proto::AreaState();
    auto path = entry.path();
    auto status = util::proto::ParseProtoFile(path.string(), state);
    if (!status.ok()) {
      return status;
    }
    stateRef[path.filename().string()] = state;
  }

  return util::OkStatus();
}

} // namespace

void Golden::Plans() {
  plans_.reset(new std::unordered_map<std::string, actions::proto::Plan*>());
}

void Golden::AreaStates() {
  area_states_.reset(
      new std::unordered_map<std::string, sevenyears::proto::AreaState*>());
}

void PopulateScenarioFiles(const std::string& location,
                           games::setup::proto::ScenarioFiles* config) {
  const std::string kBase = baseDir(location);

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

util::Status LoadGoldens(const std::string& location, Golden* golds) {
  const std::string kBase = absl::StrJoin({baseDir(location), kGoldens}, "/");
  if (!std::experimental::filesystem::exists(kBase)) {
    return util::NotFoundError(absl::Substitute("Could not find $0", kBase));
  }
  auto status = loadGoldenPlans(kBase, golds->plans_.get());
  if (!status.ok()) {
    return status;
  }
  status = loadGoldenStates(kBase, golds->area_states_.get());
  if (!status.ok()) {
    return status;
  }
  return util::OkStatus();
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
  status = games::setup::CanonicaliseWorld(&world_proto_);
  if (!status.ok()) {
    return status;
  }
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

sevenyears::proto::AreaState*
TestState::mutable_area_state(const util::proto::ObjectId& area_id) {
  if (state_map_.find(area_id) == state_map_.end()) {
    Log::Errorf("No state for area %s", util::objectid::DisplayString(area_id));
    static proto::AreaState dummy;
    return &dummy;
  }
  return &state_map_.at(area_id);
}

const industry::Production&
TestState::ProductionChain(const std::string& name) const {
  static industry::Production dummy;
  return dummy;
}

const std::string FileTag(const util::proto::ObjectId& obj_id) {
  return util::objectid::Tag(obj_id) + ".pb.txt";
}


}  // namespace sevenyears
