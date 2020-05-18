#include "games/sevenyears/merchant_ship_ai.h"

#include <memory>
#include <unordered_map>

#include "absl/strings/substitute.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/industry/industry.h"
#include "games/market/goods_utils.h"
#include "games/setup/setup.h"
#include "games/sevenyears/constants.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
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

  util::Status Initialise(const games::setup::proto::ScenarioFiles& config) {
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
  const games::setup::World& World() const override {
    return *world_;
  }
  const games::setup::Constants& Constants() const override {
    return constants_;
  }
  const proto::AreaState&
  AreaState(const util::proto::ObjectId& area_id) const {
    if (state_map_.find(area_id) == state_map_.end()) {
      Log::Errorf("No state for area %s",
                  util::objectid::DisplayString(area_id));
      static proto::AreaState dummy;
      return dummy;
    }
    return state_map_.at(area_id);
  }
  const industry::Production&
  ProductionChain(const std::string& name) const override {
    static industry::Production dummy;
    return dummy;
  }

private:
  std::unordered_map<util::proto::ObjectId, proto::AreaState> state_map_;
  std::unique_ptr<games::setup::World> world_;
  games::setup::Constants constants_;
  games::setup::proto::GameWorld world_proto_;
  games::setup::proto::Scenario scenario_proto_;
};

class SevenYearsMerchantTest : public testing::Test {
protected:
  SevenYearsMerchantTest() { Log::Register(Log::coutLogger); }
  ~SevenYearsMerchantTest() { Log::UnRegister(Log::coutLogger); }

  void SetUp() override {
    ai_proto_ = strategy_.mutable_seven_years_merchant();
  }

  util::Status LoadTestData(const std::string& location) {
    market::ClearGoods();
    const std::string kTestDir = std::getenv("TEST_SRCDIR");
    const std::string kWorkdir = std::getenv("TEST_WORKSPACE");
    const std::string kBase =
        absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, location}, "/");

    games::setup::proto::ScenarioFiles config;
    config.add_unit_templates(absl::StrJoin({kBase, kTemplates}, "/"));
    config.set_world_file(absl::StrJoin({kBase, kWorld}, "/"));
    world_state_.reset(new TestState());
    auto status = world_state_->Initialise(config);
    if (!status.ok()) {
      return status;
    }
    merchant_ai_.reset(new SevenYearsMerchant(world_state_.get()));
    return util::OkStatus();
  }

  std::unique_ptr<SevenYearsMerchant> merchant_ai_;
  std::unique_ptr<TestState> world_state_;
  actions::proto::Strategy strategy_;
  actions::proto::SevenYearsMerchant* ai_proto_;
};

TEST_F(SevenYearsMerchantTest, TestValidMission) {
  auto status = merchant_ai_->ValidMission(*ai_proto_);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("neither mission nor default"));
  ai_proto_->set_mission("bad_mission");
  status = merchant_ai_->ValidMission(*ai_proto_);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("invalid mission"));
  ai_proto_->set_mission("european_trade");
  status = merchant_ai_->ValidMission(*ai_proto_);
  EXPECT_TRUE(status.ok()) << status.error_message();
  ai_proto_->set_default_mission("bad_mission");
  status = merchant_ai_->ValidMission(*ai_proto_);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("invalid default mission"));
  ai_proto_->set_default_mission("european_trade");
  status = merchant_ai_->ValidMission(*ai_proto_);
  EXPECT_TRUE(status.ok()) << status.error_message();
}

TEST_F(SevenYearsMerchantTest, TestEuropeanTrade) {
  auto status = LoadTestData(constants::EuropeanTrade());
  ASSERT_TRUE(status.ok()) << status.error_message();
  ai_proto_->set_mission(constants::EuropeanTrade());
  auto& unit = world_state_->World().units_[0];
  status = merchant_ai_->AddStepsToPlan(*unit, unit->strategy(),
                                        unit->mutable_plan());
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(3, unit->plan().steps_size()) << unit->plan().DebugString();
  EXPECT_EQ(constants::LoadShip(), unit->plan().steps(0).key());
  EXPECT_EQ(actions::proto::AA_MOVE, unit->plan().steps(1).action());
  EXPECT_EQ(constants::EuropeanTrade(), unit->plan().steps(2).key());

  unit->mutable_plan()->clear_steps();
  unit->mutable_location()->mutable_a_area_id()->set_number(2);
  status = merchant_ai_->AddStepsToPlan(*unit, unit->strategy(),
                                        unit->mutable_plan());
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(2, unit->plan().steps_size()) << unit->plan().DebugString();
  EXPECT_EQ(actions::proto::AA_MOVE, unit->plan().steps(0).action());
  EXPECT_EQ(constants::OffloadCargo(), unit->plan().steps(1).key());
}

}  // namespace sevenyears
