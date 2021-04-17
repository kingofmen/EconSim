#include "games/ai/impl/executor_impl.h"

#include "games/actions/proto/plan.pb.h"
#include "games/ai/impl/ai_testing.h"
#include "games/ai/public/cost.h"
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

class ExecutorImplTest : public AiTestBase {};

TEST_F(ExecutorImplTest, TestMoveUnit) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_MOVE);
  step->set_connection_id(connection_12->ID());

  auto status = MoveUnit(ZeroCost(), plan_.steps(0), unit_.get());
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(unit_->location().a_area_id().number(), area2_->area_id().number());
}

TEST_F(ExecutorImplTest, TestBuy) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_BUY);
  step->set_good(kTestGood1);

  EXPECT_TRUE(BuyOrSell(ZeroCost(), plan_.steps(0), unit_.get()).ok());
  step->set_action(actions::proto::AA_SELL);
  EXPECT_TRUE(BuyOrSell(ZeroCost(), plan_.steps(0), unit_.get()).ok());
}

TEST_F(ExecutorImplTest, TestSwitchState) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_SWITCH_STATE);
  unit_->mutable_strategy()->mutable_shuttle_trade()->set_state(
      actions::proto::ShuttleTrade::STS_BUY_A);

  EXPECT_TRUE(SwitchState(ZeroCost(), *step, unit_.get()).ok());
  EXPECT_EQ(actions::proto::ShuttleTrade::STS_BUY_Z,
            unit_->strategy().shuttle_trade().state());
}

TEST_F(ExecutorImplTest, TestTurnAround) {
  auto* step = plan_.add_steps();
  int kStartProgress = 10000;
  step->set_action(actions::proto::AA_TURN_AROUND);
  unit_->mutable_location()->set_connection_id(connection_12->connection_id());
  unit_->mutable_location()->set_progress_u(kStartProgress);
  auto status = TurnAround(ZeroCost(), *step, unit_.get());
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(connection_12->length_u() - kStartProgress,
            unit_->location().progress_u());
  EXPECT_TRUE(unit_->location().a_area_id() == area2_->area_id());
}

} // namespace impl
} // namespace ai
