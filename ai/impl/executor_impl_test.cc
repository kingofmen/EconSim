#include "executor_impl.h"

#include "actions/proto/plan.pb.h"
#include "geography/connection.h"
#include "geography/geography.h"
#include "geography/proto/geography.pb.h"
#include "gtest/gtest.h"
#include "units/unit.h"
#include "units/proto/templates.pb.h"
#include "units/proto/units.pb.h"

namespace ai {
namespace impl {
namespace {

constexpr char kTestGood1[] = "test_good_1";

} // namespace

class ExecutorImplTest : public testing::Test {
 protected:
  void SetUp() override {
    geography::proto::Area area;
    area.set_id(1);
    area1_ = geography::Area::FromProto(area);
    area.set_id(2);
    area2_ = geography::Area::FromProto(area);

    geography::proto::Connection conn;
    conn.set_id(1);
    conn.set_a(1);
    conn.set_z(2);
    conn.set_distance_u(1);
    conn.set_width_u(1);
    connection_12 = geography::Connection::FromProto(conn);

    units::proto::Template temp;
    temp.set_id(1);
    temp.mutable_mobility()->set_speed_u(1);
    units::Unit::RegisterTemplate(temp);

    units::proto::Unit unit;
    unit.mutable_unit_id()->set_number(1);
    unit.mutable_unit_id()->set_type(1);
    unit.mutable_location()->set_source_area_id(1);
    unit_ = units::Unit::FromProto(unit);
  }
  
  actions::proto::Plan plan_;
  std::unique_ptr<geography::Area> area1_;
  std::unique_ptr<geography::Area> area2_;
  std::unique_ptr<geography::Connection> connection_12;
  std::unique_ptr<units::Unit> unit_;
};

TEST_F(ExecutorImplTest, TestMoveUnit) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_MOVE);
  step->set_connection_id(connection_12->ID());

  EXPECT_TRUE(MoveUnit(plan_.steps(0), unit_.get()));
  EXPECT_EQ(2, unit_->location().source_area_id());
}

TEST_F(ExecutorImplTest, TestBuy) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_BUY);
  step->set_good(kTestGood1);

  EXPECT_TRUE(BuyOrSell(plan_.steps(0), unit_.get()));
  step->set_action(actions::proto::AA_SELL);
  EXPECT_TRUE(BuyOrSell(plan_.steps(0), unit_.get()));
}

TEST_F(ExecutorImplTest, TestSwitchState) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_SWITCH_STATE);
  unit_->mutable_strategy()->mutable_shuttle_trade()->set_state(
      actions::proto::ShuttleTrade::STS_BUY_A);

  EXPECT_TRUE(SwitchState(*step, unit_.get()));
  EXPECT_EQ(actions::proto::ShuttleTrade::STS_BUY_Z,
            unit_->strategy().shuttle_trade().state());
}

} // namespace impl
} // namespace ai
