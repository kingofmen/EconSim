#include "games/ai/impl/ai_utils.h"

#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "gtest/gtest.h"

namespace ai {
namespace utils {

class AiUtilsTest : public testing::Test {
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
    conn.set_distance_u(micro::kOneInU);
    conn.set_width_u(1);
    connection_12 = geography::Connection::FromProto(conn);

    units::proto::Template temp;
    temp.mutable_template_id()->set_kind("one");
    temp.mutable_mobility()->set_speed_u(1000000);
    temp.set_base_action_points_u(1000000);
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

TEST_F(AiUtilsTest, TestNumTurns) {
  std::vector<uint64> path;
  path.push_back(connection_12->connection_id());
  EXPECT_EQ(1, NumTurns(*unit_, path));
}

} // namespace impl
} // namespace ai
