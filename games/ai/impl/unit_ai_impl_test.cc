#include "games/ai/impl/unit_ai_impl.h"

#include "games/actions/proto/strategy.pb.h"
#include "games/actions/proto/plan.pb.h"
#include "games/ai/impl/ai_testing.h"
#include "games/ai/planner.h"
#include "games/geography/connection.h"
#include "games/geography/geography.h"
#include "games/geography/mobile.h"
#include "games/geography/proto/geography.pb.h"
#include "games/market/goods_utils.h"
#include "games/units/unit.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "gtest/gtest.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"
#include "util/logging/logging.h"

namespace ai {
namespace impl {
namespace {
constexpr char kTestGood1[] = "test1";
constexpr char kTestGood2[] = "test2";
}

class UnitAiImplTest : public AiTestBase {};

TEST_F(UnitAiImplTest, TestShuttleTrader) {
  ShuttleTrader trader;
  actions::proto::ShuttleTrade* trade = strategy_.mutable_shuttle_trade();
  trade->set_good_a(kTestGood1);
  trade->set_good_z(kTestGood2);
  *trade->mutable_area_a_id() = area1_->area_id();
  *trade->mutable_area_z_id() = area3_->area_id();
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

TEST_F(UnitAiImplTest, TestFindPath) {
  actions::proto::Plan plan;
  // Path from A to Z.
  auto status = FindPath(*unit_, ShortestDistance, ZeroHeuristic,
                         area3_->area_id(), &plan);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(plan.steps_size(), 2);

  // Path starting in connection.
  plan.Clear();
  unit_->mutable_location()->set_connection_id(connection_12->connection_id());
  unit_->mutable_location()->set_progress_u(micro::kHalfInU);
  status = FindPath(*unit_, ShortestDistance, ZeroHeuristic, area3_->area_id(),
                    &plan);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(plan.steps_size(), 2);

  // Path starting in connection, but we have to backtrack.
  plan.Clear();
  unit_->mutable_location()->set_connection_id(connection_12->connection_id());
  unit_->mutable_location()->set_progress_u(micro::kHalfInU);
  status = FindPath(*unit_, ShortestDistance, ZeroHeuristic, area4_->area_id(),
                    &plan);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(plan.steps_size(), 3);

  // Return to A location when we've made some progress.
  plan.Clear();
  unit_->mutable_location()->set_connection_id(connection_12->connection_id());
  unit_->mutable_location()->set_progress_u(micro::kHalfInU);
  status = FindPath(*unit_, ShortestDistance, ZeroHeuristic, area1_->area_id(),
                    &plan);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(plan.steps_size(), 2);
  if (plan.steps_size() == 2) {
    EXPECT_EQ(plan.steps(0).action(), actions::proto::AA_TURN_AROUND);
    EXPECT_EQ(plan.steps(1).action(), actions::proto::AA_MOVE);
  }
}

} // namespace impl
} // namespace ai
