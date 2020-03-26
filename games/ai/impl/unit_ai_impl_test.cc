#include "games/ai/impl/unit_ai_impl.h"

#include "actions/proto/strategy.pb.h"
#include "actions/proto/plan.pb.h"
#include "games/ai/planner.h"
#include "geography/connection.h"
#include "geography/geography.h"
#include "geography/mobile.h"
#include "geography/proto/geography.pb.h"
#include "gtest/gtest.h"
#include "market/goods_utils.h"
#include "units/unit.h"
#include "units/proto/templates.pb.h"
#include "units/proto/units.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"

namespace ai {
namespace impl {
namespace {
constexpr char kTestGood1[] = "test1";
constexpr char kTestGood2[] = "test2";
}

class UnitAiImplTest : public testing::Test {
 protected:
  void SetUp() override {
    geography::proto::Area area;
    area.set_id(1);
    area1_ = geography::Area::FromProto(area);
    area.set_id(2);
    area2_ = geography::Area::FromProto(area);
    area.set_id(3);
    area3_ = geography::Area::FromProto(area);

    geography::proto::Connection conn;
    conn.set_id(1);
    conn.set_a(1);
    conn.set_z(2);
    conn.set_distance_u(1);
    conn.set_width_u(1);
    connection_12 = geography::Connection::FromProto(conn);
    conn.set_id(2);
    conn.set_a(3);
    conn.set_z(2);
    connection_23 = geography::Connection::FromProto(conn);

    units::proto::Template temp;
    temp.set_id(1);
    units::Unit::RegisterTemplate(temp);

    units::proto::Unit unit;
    unit.mutable_unit_id()->set_number(1);
    unit.mutable_unit_id()->set_type(1);
    unit.mutable_location()->set_source_area_id(area1_->id());
    unit_ = units::Unit::FromProto(unit);
  }

  std::unique_ptr<geography::Connection> connection_12;
  std::unique_ptr<geography::Connection> connection_23;
  std::unique_ptr<geography::Area> area1_;
  std::unique_ptr<geography::Area> area2_;
  std::unique_ptr<geography::Area> area3_;
  std::unique_ptr<units::Unit> unit_;
  actions::proto::Strategy strategy_;
};

TEST_F(UnitAiImplTest, TestShuttleTrader) {
  ShuttleTrader trader;
  actions::proto::ShuttleTrade* trade = strategy_.mutable_shuttle_trade();
  trade->set_good_a(kTestGood1);
  trade->set_good_z(kTestGood2);
  trade->set_area_a_id(area1_->id());
  trade->set_area_z_id(area3_->id());
  trade->set_state(actions::proto::ShuttleTrade::STS_BUY_A);

  actions::proto::Plan plan;
  auto status = ai::MakePlan(*unit_, strategy_, &plan);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(3, plan.steps_size());
  if (plan.steps_size() == 3) {
    EXPECT_EQ(actions::proto::AA_SELL, plan.steps(0).action());
    EXPECT_EQ(kTestGood2, plan.steps(0).good());
    EXPECT_EQ(actions::proto::AA_BUY, plan.steps(1).action());
    EXPECT_EQ(kTestGood1, plan.steps(1).good());
    EXPECT_EQ(actions::proto::AA_SWITCH_STATE, plan.steps(2).action());
  }

  market::SetAmount(kTestGood1, micro::kOneInU, unit_->mutable_resources());
  market::SetAmount(kTestGood2, 0, unit_->mutable_resources());
  trade->set_state(actions::proto::ShuttleTrade::STS_BUY_Z);

  plan.Clear();
  status = ai::MakePlan(*unit_, strategy_, &plan);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(5, plan.steps_size());
  if (plan.steps_size() >= 2) {
    EXPECT_EQ(actions::proto::AA_MOVE, plan.steps(0).action());
    EXPECT_EQ(connection_12->ID(), plan.steps(0).connection_id());
    EXPECT_EQ(actions::proto::AA_MOVE, plan.steps(1).action());
    EXPECT_EQ(connection_23->ID(), plan.steps(1).connection_id());
  }
}

} // namespace impl
} // namespace ai
