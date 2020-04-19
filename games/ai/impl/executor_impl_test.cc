#include "games/ai/impl/executor_impl.h"

#include "games/actions/proto/plan.pb.h"
#include "games/geography/connection.h"
#include "games/geography/geography.h"
#include "games/geography/proto/geography.pb.h"
#include "games/units/unit.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "gtest/gtest.h"

namespace ai {
namespace impl {
namespace {

constexpr char kTestGood1[] = "test_good_1";

} // namespace

class ExecutorImplTest : public testing::Test {
 protected:
  void SetUp() override {
    geography::proto::Area area;
    area.mutable_area_id()->set_number(1);
    area1_ = geography::Area::FromProto(area);
    area.mutable_area_id()->set_number(2);
    area2_ = geography::Area::FromProto(area);

    geography::proto::Connection conn;
    conn.set_id(1);
    conn.mutable_a_area_id()->set_number(1);
    conn.mutable_z_area_id()->set_number(2);
    conn.set_distance_u(1);
    conn.set_width_u(1);
    connection_12 = geography::Connection::FromProto(conn);

    units::proto::Template temp;
    temp.mutable_template_id()->set_kind("one");
    temp.mutable_mobility()->set_speed_u(1);
    units::Unit::RegisterTemplate(temp);

    units::proto::Unit unit;
    unit.mutable_unit_id()->set_kind("one");
    *unit.mutable_location()->mutable_a_area_id() = area1_->area_id();
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

  EXPECT_TRUE(MoveUnit(plan_.steps(0), unit_.get()).ok());
  EXPECT_TRUE(unit_->location().a_area_id() == area2_->area_id());
}

TEST_F(ExecutorImplTest, TestBuy) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_BUY);
  step->set_good(kTestGood1);

  EXPECT_TRUE(BuyOrSell(plan_.steps(0), unit_.get()).ok());
  step->set_action(actions::proto::AA_SELL);
  EXPECT_TRUE(BuyOrSell(plan_.steps(0), unit_.get()).ok());
}

TEST_F(ExecutorImplTest, TestSwitchState) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_SWITCH_STATE);
  unit_->mutable_strategy()->mutable_shuttle_trade()->set_state(
      actions::proto::ShuttleTrade::STS_BUY_A);

  EXPECT_TRUE(SwitchState(*step, unit_.get()).ok());
  EXPECT_EQ(actions::proto::ShuttleTrade::STS_BUY_Z,
            unit_->strategy().shuttle_trade().state());
}

} // namespace impl
} // namespace ai
