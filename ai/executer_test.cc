#include "executer.h"

#include "actions/proto/plan.pb.h"
#include "gtest/gtest.h"
#include "units/unit.h"
#include "units/proto/templates.pb.h"
#include "units/proto/units.pb.h"

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
}

TEST_F(ExecuterTest, TestExecuteStep) {
  EXPECT_FALSE(ExecuteStep(plan_, NULL));
  auto* step = plan_.add_steps();
  EXPECT_FALSE(ExecuteStep(plan_, NULL));
}


} // namespace impl
} // namespace ai
