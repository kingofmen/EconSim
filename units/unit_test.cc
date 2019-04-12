#include "units/unit.h"

#include <memory>

#include "gtest/gtest.h"
#include "units/proto/templates.pb.h"
#include "units/proto/units.pb.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.h"

namespace units {

// UnitTest, lol.
class UnitTest : public testing::Test {
 protected:
  void SetUp() override {
    template_.set_id(1);
    template_.mutable_mobility()->set_speed_u(1);
    template_.set_name("test_unit");
    Unit::RegisterTemplate(template_);

    unit_proto_.mutable_unit_id()->set_type(1);
    unit_proto_.mutable_unit_id()->set_number(1);
    unit_ = Unit::FromProto(unit_proto_);
  }

  proto::Template template_;
  proto::Unit unit_proto_;
  std::unique_ptr<Unit> unit_;
};

TEST_F(UnitTest, GetById) {
  util::proto::ObjectId id;
  id.set_type(1);
  id.set_number(1);
  EXPECT_EQ(unit_.get(), Unit::ById(id));
}


} // namespace units

