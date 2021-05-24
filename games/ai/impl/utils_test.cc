#include "games/ai/impl/ai_utils.h"

#include "games/ai/impl/ai_testing.h"
#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "gtest/gtest.h"

namespace ai {
namespace utils {

class AiUtilsTest : public AiTestBase {};

TEST_F(AiUtilsTest, TestNumTurns) {
  std::vector<geography::Connection::IdType> path;
  path.push_back(connection_12->connection_id());
  EXPECT_EQ(1, NumTurns(*unit_, path));
  path.push_back(connection_23->connection_id());
  EXPECT_EQ(2, NumTurns(*unit_, path));
}

TEST_F(AiUtilsTest, TestLowerSpeed) {
  units::proto::Template temp;
  temp.mutable_template_id()->set_kind("slow");
  temp.mutable_mobility()->set_speed_u(micro::kTwoThirdsInU);
  temp.set_base_action_points_u(micro::kOneInU);
  units::Unit::RegisterTemplate(temp);

  units::proto::Unit unit;
  unit.mutable_unit_id()->set_kind("slow");
  unit.mutable_unit_id()->set_number(2);
  *unit.mutable_location()->mutable_a_area_id() = area1_->area_id();
  unit_ = units::Unit::FromProto(unit);
  std::vector<geography::Connection::IdType> path;
  path.push_back(connection_12->connection_id());
  EXPECT_EQ(2, NumTurns(*unit_, path));
  path.push_back(connection_23->connection_id());
  // TODO(Issue 33, issue 34): This should be 3 when these issues are fixed.
  EXPECT_EQ(4, NumTurns(*unit_, path));
}

} // namespace impl
} // namespace ai
