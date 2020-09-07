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
#include "games/sevenyears/test_utils.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/google/protobuf/util/message_differencer.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

class SevenYearsMerchantTest : public testing::Test {
protected:
  SevenYearsMerchantTest() { Log::Register(Log::coutLogger); }
  ~SevenYearsMerchantTest() { Log::UnRegister(Log::coutLogger); }

  void SetUp() override {
    market::ClearGoods();
    ai_proto_ = strategy_.mutable_seven_years_merchant();
    world_state_.reset(new TestState());
  }

  util::Status LoadTestData(const std::string& location) {
    auto status = world_state_->Initialise(location);
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
  Golden golds;
  golds.Plans();
  status = LoadGoldens(constants::EuropeanTrade(), &golds);
  EXPECT_OK(status) << "Could not load golden files: "
                    << status.error_message();
  ai_proto_->set_mission(constants::EuropeanTrade());

  google::protobuf::util::MessageDifferencer differ;
  for (auto& unit : world_state_->World().units_) {
    status = merchant_ai_->AddStepsToPlan(*unit, unit->strategy(),
                                          unit->mutable_plan());
    EXPECT_OK(status) << status.error_message();
    auto unitTag = FileTag(unit->unit_id());
    auto goldIt = golds.plans_->find(unitTag);
    if (goldIt == golds.plans_->end()) {
      FAIL() << "Could not find golden plan for unit "
             << util::objectid::DisplayString(unit->unit_id()) << " ("
             << unitTag << ")";
      continue;
    }

    auto* goldPlan = goldIt->second;

    EXPECT_TRUE(differ.Equals(*goldPlan, unit->plan()))
        << util::objectid::DisplayString(unit->unit_id()) << ": Golden plan "
        << goldPlan->DebugString() << "\ndiffers from found plan\n"
        << unit->plan().DebugString();
  }

  /*
  auto& unit = world_state_->World().units_[0];
  status = merchant_ai_->AddStepsToPlan(*unit, unit->strategy(),
                                        unit->mutable_plan());
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(4, unit->plan().steps_size()) << unit->plan().DebugString();
  EXPECT_EQ(constants::LoadShip(), unit->plan().steps(0).key());
  EXPECT_EQ(constants::TradeGoods(), unit->plan().steps(0).good());
  EXPECT_EQ(actions::proto::AA_MOVE, unit->plan().steps(1).action());
  EXPECT_EQ(constants::OffloadCargo(), unit->plan().steps(2).key());
  EXPECT_EQ(constants::LoadShip(), unit->plan().steps(3).key());
  EXPECT_EQ(constants::Supplies(), unit->plan().steps(3).good());

  unit->mutable_plan()->clear_steps();
  unit->mutable_location()->mutable_a_area_id()->set_number(2);
  status = merchant_ai_->AddStepsToPlan(*unit, unit->strategy(),
                                        unit->mutable_plan());
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(2, unit->plan().steps_size()) << unit->plan().DebugString();
  EXPECT_EQ(actions::proto::AA_MOVE, unit->plan().steps(0).action());
  EXPECT_EQ(constants::OffloadCargo(), unit->plan().steps(1).key());
  */
}

}  // namespace sevenyears
