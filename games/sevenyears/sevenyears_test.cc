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

TEST_F(SevenYearsTest, TestExecutors) {
  auto status = LoadTestData("executors");
  ASSERT_TRUE(status.ok()) << status.error_message();
  status = game_->InitialiseAI();
  ASSERT_TRUE(status.ok()) << status.error_message();

  game_->NewTurn();
  for (auto& unit : game_->World().units_) {
    EXPECT_EQ(0, unit->plan().steps_size())
        << absl::Substitute("$0 has unexpected step remaining: $1",
                            util::objectid::DisplayString(unit->unit_id()),
                            unit->plan().DebugString());
    // TODO: Don't hardcode unit numbers here, make a proper golden-state proto.
    switch (unit->unit_id().number()) {
      case 1:
        EXPECT_EQ(2 * micro::kOneInU,
                  market::GetAmount(unit->resources(), constants::TradeGoods()));
        break;
      case 2:
        EXPECT_EQ(0,
                  market::GetAmount(unit->resources(), constants::Supplies()));
        break;
      default:
        break;
    }
  }
}


}  // namespace sevenyears
