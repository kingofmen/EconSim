#include "games/ai/impl/ai_testing.h"

#include "games/actions/proto/strategy.pb.h"
#include "games/actions/proto/plan.pb.h"
#include "games/geography/connection.h"
#include "games/geography/geography.h"
#include "games/geography/proto/geography.pb.h"
#include "games/units/unit.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"
#include "util/logging/logging.h"

namespace ai {

void AiTestBase::SetUp() {
  Log::Register(Log::coutLogger);
  // 4 <-> 1 <-> 2 <-> 3
  geography::proto::Area area;
  area.mutable_area_id()->set_kind("area");
  area.mutable_area_id()->set_number(1);
  area1_ = geography::Area::FromProto(area);
  area.mutable_area_id()->set_number(2);
  area2_ = geography::Area::FromProto(area);
  area.mutable_area_id()->set_number(3);
  area3_ = geography::Area::FromProto(area);
  area.mutable_area_id()->set_number(4);
  area4_ = geography::Area::FromProto(area);

  geography::proto::Connection conn;
  conn.set_id(1);
  conn.mutable_a_area_id()->set_kind("area");
  conn.mutable_z_area_id()->set_kind("area");
  conn.mutable_a_area_id()->set_number(1);
  conn.mutable_z_area_id()->set_number(2);
  conn.set_distance_u(micro::kOneInU);
  conn.set_width_u(1);
  connection_12 = geography::Connection::FromProto(conn);
  conn.set_id(2);
  conn.mutable_a_area_id()->set_number(3);
  conn.mutable_z_area_id()->set_number(2);
  connection_23 = geography::Connection::FromProto(conn);
  conn.set_id(3);
  conn.mutable_a_area_id()->set_number(1);
  conn.mutable_z_area_id()->set_number(4);
  connection_14 = geography::Connection::FromProto(conn);

  units::proto::Template temp;
  temp.mutable_template_id()->set_kind("one");
  temp.mutable_mobility()->set_speed_u(micro::kOneInU);
  temp.set_base_action_points_u(micro::kOneInU);
  units::Unit::RegisterTemplate(temp);

  units::proto::Unit unit;
  unit.mutable_unit_id()->set_kind("one");
  unit.mutable_unit_id()->set_number(1);
  *unit.mutable_location()->mutable_a_area_id() = area1_->area_id();
  unit_ = units::Unit::FromProto(unit);
}

void AiTestBase::TearDown() { Log::UnRegister(Log::coutLogger); }

} // namespace ai


