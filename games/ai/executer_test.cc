#include "games/ai/executer.h"

#include "games/actions/proto/plan.pb.h"
#include "games/units/unit.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "games/units/unit.h"
#include "util/proto/object_id.h"
#include "gmock/gmock.h"
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
  *step->mutable_connection_id() = util::objectid::New("connection", 1);
  DeleteStep(&plan_);

  EXPECT_EQ(0, plan_.steps_size());
  step = plan_.add_steps();
  step = plan_.add_steps();
  EXPECT_EQ(2, plan_.steps_size());
  DeleteStep(&plan_);
  EXPECT_EQ(1, plan_.steps_size());
}

TEST_F(ExecuterTest, TestExecuteStep) {
  auto status = ExecuteStep(plan_, nullptr);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("No steps"))
      << status.error_message();

  auto* step = plan_.add_steps();
  status = ExecuteStep(plan_, nullptr);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("Null unit"))
      << status.error_message();

  units::proto::Template temp;
  temp.mutable_template_id()->set_kind("template");
  temp.set_base_action_points_u(1);
  units::Unit::RegisterTemplate(temp);
  units::proto::Unit unit_proto;
  unit_proto.mutable_unit_id()->set_kind("template");
  unit_proto.mutable_unit_id()->set_number(1);
  auto unit = units::Unit::FromProto(unit_proto);

  status = ExecuteStep(plan_, unit.get());
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("neither action or key"))
      << status.error_message();

  step->set_action(actions::proto::AA_UNKNOWN);
  status = ExecuteStep(plan_, unit.get());
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("not implemented"))
      << status.error_message();

  bool touched = false;
  StepExecutor noop = [&touched](const ActionCost&, const actions::proto::Step&,
                                 units::Unit*) {
    touched = true;
    return util::OkStatus();
  };

  RegisterExecutor(actions::proto::AA_UNKNOWN, noop);
  status = ExecuteStep(plan_, unit.get());
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_TRUE(touched);

  step->set_key("test");
  status = ExecuteStep(plan_, unit.get());
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("not implemented"))
      << status.error_message();

  touched = false;
  RegisterExecutor("test", noop);
  status = ExecuteStep(plan_, unit.get());
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_TRUE(touched);

  uint64 current_cost = 10;
  CostCalculator cost = [&current_cost](const actions::proto::Step&,
                                        const units::Unit&) {
    return ActionCost(current_cost, micro::kOneInU);
  };
  RegisterCost(step->key(), cost);

  touched = false;
  status = ExecuteStep(plan_, unit.get());
  EXPECT_FALSE(touched);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("action points"))
      << status.error_message();
  EXPECT_EQ(1, unit->action_points_u());

  current_cost = 1;
  status = ExecuteStep(plan_, unit.get());
  EXPECT_TRUE(touched);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(0, unit->action_points_u());
}


} // namespace impl
} // namespace ai
