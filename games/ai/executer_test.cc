#include "games/ai/executer.h"

#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "gtest/gtest.h"

namespace ai {
namespace impl {

class ExecuterTest : public testing::Test {
 protected:
  void SetUp() override {
  }
  
  actions::proto::Plan plan_;
};

TEST_F(ExecuterTest, TestDeleteStep) {
  auto* step = plan_.add_steps();
  step->set_action(actions::proto::AA_MOVE);
  step->set_connection_id(1);
  DeleteStep(&plan_);

  EXPECT_EQ(0, plan_.steps_size());
  step = plan_.add_steps();
  step = plan_.add_steps();
  EXPECT_EQ(2, plan_.steps_size());
  DeleteStep(&plan_);
  EXPECT_EQ(1, plan_.steps_size());
}

TEST_F(ExecuterTest, TestExecuteStep) {
  EXPECT_FALSE(ExecuteStep(plan_, NULL).ok());
  auto* step = plan_.add_steps();
  EXPECT_FALSE(ExecuteStep(plan_, NULL).ok());
}


} // namespace impl
} // namespace ai
