#include "games/sevenyears/test_utils.h"

#include <experimental/filesystem>

#include "absl/strings/substitute.h"
#include "games/industry/industry.h"
#include "games/setup/setup.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/google/protobuf/util/message_differencer.h"
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
const std::string kUnits = "units";
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
    std::unordered_map<std::string,
                       sevenyears::testdata::proto::AreaStateList*>* states) {
  if (states == nullptr) {
    return util::OkStatus();
  }

  auto dir = absl::StrJoin({base, kStates}, "/");
  if (!std::experimental::filesystem::exists(dir)) {
    return util::NotFoundError(absl::Substitute("Could not find $0", dir));
  }

  auto& stateRef = *states;
  for (auto& entry : std::experimental::filesystem::directory_iterator(dir)) {
    auto* state = new sevenyears::testdata::proto::AreaStateList();
    auto path = entry.path();
    auto status = util::proto::ParseProtoFile(path.string(), state);
    if (!status.ok()) {
      return status;
    }
    stateRef[path.filename().string()] = state;
  }

  return util::OkStatus();
}

util::Status loadGoldenUnits(
    const std::string& base,
    std::unordered_map<std::string,
                       sevenyears::testdata::proto::UnitStateList*>* states) {
  if (states == nullptr) {
    return util::OkStatus();
  }

  auto dir = absl::StrJoin({base, kUnits}, "/");
  if (!std::experimental::filesystem::exists(dir)) {
    return util::NotFoundErrorf("Could not find %s", dir);
  }

  auto& stateRef = *states;
  for (auto& entry : std::experimental::filesystem::directory_iterator(dir)) {
    auto* state = new sevenyears::testdata::proto::UnitStateList();
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
      new std::unordered_map<std::string,
                             sevenyears::testdata::proto::AreaStateList*>());
}

void Golden::Units() {
  unit_states_.reset(
      new std::unordered_map<std::string,
                             sevenyears::testdata::proto::UnitStateList*>());
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

util::Status
TestState::PopulateProtos(const games::setup::proto::ScenarioFiles& config) {
  auto status = games::setup::LoadScenario(config, &scenario_proto_);
  if (!status.ok()) {
    return status;
  }
  status = games::setup::LoadWorld(config, &world_proto_);
  if (!status.ok()) {
    return status;
  }
  constants_ = games::setup::Constants(scenario_proto_);
  return util::OkStatus();
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
  status = loadGoldenUnits(kBase, golds->unit_states_.get());
  if (!status.ok()) {
    return status;
  }
  return util::OkStatus();
}

util::Status
TestState::Initialise(const games::setup::proto::ScenarioFiles& config) {
  auto status = PopulateProtos(config);
  if (!status.ok()) {
    return status;
  }
  status = games::setup::CanonicaliseWorld(&world_proto_);
  if (!status.ok()) {
    return status;
  }
  game_world_ = games::setup::World::FromProto(world_proto_);
  sevenyears::proto::WorldState* world_state = world_proto_.MutableExtension(
      sevenyears::proto::WorldState::sevenyears_state);
  for (const auto& as : world_state->area_states()) {
    area_states_[as.area_id()] = as;
  }
  setTime(world_state->timestamp());

  return util::OkStatus();
}

util::Status
TestState::Initialise(const std::string& location) {
  games::setup::proto::ScenarioFiles config;
  PopulateScenarioFiles(location, &config);
  return Initialise(config);
}

const industry::Production&
TestState::ProductionChain(const std::string& name) const {
  static industry::Production dummy;
  return dummy;
}

const std::string FileTag(const util::proto::ObjectId& obj_id) {
  return util::objectid::Tag(obj_id) + ".pb.txt";
}

std::vector<const units::Unit*>
TestState::ListUnits(const units::Filter& filter) const {
  std::vector<const units::Unit*> result;
  // TODO: Implement filtering.
  for (const auto& uptr : game_world_->units_) {
    result.push_back(uptr.get());
  }
  return result;
}


void CheckAreaStatesForStage(const SevenYearsState& got, const Golden& want,
                             int stage) {
  google::protobuf::util::MessageDifferencer differ;
  if (want.area_states_->empty()) {
    EXPECT_FALSE(want.area_states_->empty()) << "No golden area states.";
    return;
  }
  for (const auto& goldIt : *(want.area_states_)) {
    std::string tag = goldIt.first;
    const auto* goldStateList = goldIt.second;
    if (goldStateList->states_size() <= stage) {
      EXPECT_GT(goldStateList->states_size(), stage);
      continue;
    }
    const auto& goldState = goldStateList->states(stage);
    const util::proto::ObjectId& area_id = goldState.area_id();
    const auto& actual = got.AreaState(area_id);
    EXPECT_TRUE(differ.Equals(goldState, actual))
        << "Stage " << stage << ": " << util::objectid::DisplayString(area_id)
        << ": Golden state " << goldState.DebugString()
        << "\ndiffers from actual state\n"
        << actual.DebugString();
  }
}

void CheckUnitStatesForStage(const SevenYearsState& got, const Golden& want,
                             int stage) {
  google::protobuf::util::MessageDifferencer differ;
  if (want.unit_states_->empty()) {
    EXPECT_FALSE(want.unit_states_->empty()) << "No golden unit states.";
    return;
  }
  for (const auto& goldIt : *(want.unit_states_)) {
    std::string tag = goldIt.first;
    const auto* goldStateList = goldIt.second;
    if (goldStateList->states_size() <= stage) {
      EXPECT_GT(goldStateList->states_size(), stage);
      continue;
    }
    const auto& goldState = goldStateList->states(stage);
    const util::proto::ObjectId& unit_id = goldState.unit_id();
    bool found = false;
    for (const auto& cand : got.World().units_) {
      if (cand->unit_id() != unit_id) {
        continue;
      }
      EXPECT_TRUE(differ.Equals(goldState, cand->Proto()))
          << "Stage " << stage << ": " << util::objectid::DisplayString(unit_id)
          << ": Golden state " << goldState.DebugString()
          << "\ndiffers from actual state\n"
          << cand->Proto().DebugString();
      found = true;
      break;
    }
    if (!found) {
      EXPECT_TRUE(found) << "Did not find "
                         << util::objectid::DisplayString(unit_id)
                         << " in stage " << stage;
    }
  }
}

}  // namespace sevenyears
