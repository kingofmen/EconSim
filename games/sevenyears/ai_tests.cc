#include "games/sevenyears/merchant_ship_ai.h"

#include <memory>
#include <unordered_map>

#include "games/actions/proto/strategy.pb.h"
#include "games/setup/setup.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

class FakeState : public SevenYearsState {
public:
  FakeState() {}
  const games::setup::World& World() const override {
    return *fake_world_;
  }
  const games::setup::Constants& Constants() const override {
    return fake_constants_;
  }
  const proto::AreaState&
  AreaState(const util::proto::ObjectId& area_id) const {
    return state_map_.at(area_id);
  }

private:
  std::unordered_map<util::proto::ObjectId, proto::AreaState> state_map_;
  std::unique_ptr<games::setup::World> fake_world_;
  games::setup::Constants fake_constants_;
};

class SevenYearsMerchantTest : public testing::Test {
protected:
  void SetUp() override {
    fake_state_.reset(new FakeState());
    merchant_ai_.reset(new SevenYearsMerchant(fake_state_.get()));
  }

  std::unique_ptr<SevenYearsMerchant> merchant_ai_;
  std::unique_ptr<FakeState> fake_state_;
};

TEST_F(SevenYearsMerchantTest, TestValidMission) {
  actions::proto::SevenYearsMerchant sym;
  auto status = merchant_ai_->ValidMission(sym);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("neither mission nor default"));
  sym.set_mission("bad_mission");
  status = merchant_ai_->ValidMission(sym);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("invalid mission"));
  sym.set_mission("europe_trade");
  status = merchant_ai_->ValidMission(sym);
  EXPECT_TRUE(status.ok()) << status.error_message();
  sym.set_default_mission("bad_mission");
  status = merchant_ai_->ValidMission(sym);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("invalid default mission"));
  sym.set_default_mission("europe_trade");
  status = merchant_ai_->ValidMission(sym);
  EXPECT_TRUE(status.ok()) << status.error_message();
}


}  // namespace sevenyears
