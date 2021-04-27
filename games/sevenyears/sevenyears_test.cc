#include "games/sevenyears/sevenyears.h"

#include "absl/strings/substitute.h"
#include "games/actions/proto/strategy.pb.h"
#include "games/industry/industry.h"
#include "games/market/goods_utils.h"
#include "games/setup/setup.h"
#include "games/sevenyears/constants.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "games/sevenyears/test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

class SevenYearsTest : public testing::Test {
protected:
  SevenYearsTest() { Log::Register(Log::coutLogger); }
  ~SevenYearsTest() { Log::UnRegister(Log::coutLogger); }

  void SetUp() override {
    units::Unit::ClearTemplates();
    market::ClearGoods();
  }

  util::Status LoadTestData(const std::string& location) {
    game_.reset(new SevenYears());
    games::setup::proto::ScenarioFiles config;
    PopulateScenarioFiles(location, &config);
    return game_->LoadScenario(config);
  }

  std::unique_ptr<sevenyears::SevenYears> game_;
};

TEST_F(SevenYearsTest, Executors) {
  const std::string testName("executors");
  auto status = LoadTestData(testName);
  ASSERT_TRUE(status.ok()) << status.error_message();
  status = game_->InitialiseAI();
  ASSERT_TRUE(status.ok()) << status.error_message();

  int numStages = 1;
  Golden golds;
  golds.Units();
  status = LoadGoldens(testName, &golds);
  if (!status.ok()) {
    ASSERT_TRUE(status.ok()) << status.error_message();
  }
  for (const auto& as : *golds.unit_states_) {
    if (as.second->states_size() > numStages) {
      numStages = as.second->states_size();
    }
  }
  for (int stage = 0; stage < numStages; ++stage) {
    game_->NewTurn();
    CheckUnitStatesForStage(*game_, golds, stage);
  }
}

// End-to-end test for European trade.
TEST_F(SevenYearsTest, EuropeanTradeE2E) {
  const std::string testName("european_trade_e2e");
  auto status = LoadTestData(testName);
  ASSERT_TRUE(status.ok()) << status.error_message();
  status = game_->InitialiseAI();
  ASSERT_TRUE(status.ok()) << status.error_message();

  Golden golds;
  golds.AreaStates();
  status = LoadGoldens(testName, &golds);
  if (!status.ok()) {
    ASSERT_TRUE(status.ok()) << status.error_message();
  }
  int numStages = 0;
  for (const auto& as : *golds.area_states_) {
    if (as.second->states_size() > numStages) {
      numStages = as.second->states_size();
    }
  }
  for (int stage = 0; stage < numStages; ++stage) {
    game_->NewTurn();
    CheckAreaStatesForStage(*game_, golds, stage);
  }
}

// Test for attrition and supply consumption.
TEST_F(SevenYearsTest, ConsumeSupplies) {
  const std::string testName("attrition");
  auto status = LoadTestData(testName);
  ASSERT_TRUE(status.ok()) << status.error_message();

  int numStages = 1;
  Golden golds;
  golds.Units();
  status = LoadGoldens(testName, &golds);
  if (!status.ok()) {
    ASSERT_TRUE(status.ok()) << status.error_message();
  }
  for (const auto& as : *golds.unit_states_) {
    if (as.second->states_size() > numStages) {
      numStages = as.second->states_size();
    }
  }
  for (int stage = 0; stage < numStages; ++stage) {
    game_->consumeSupplies();
    CheckUnitStatesForStage(*game_, golds, stage);
  }

}

}  // namespace sevenyears
