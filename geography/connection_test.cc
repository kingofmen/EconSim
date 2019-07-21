#include "geography/connection.h"

#include <memory>

#include "geography/geography.h"
#include "geography/proto/geography.pb.h"
#include "gtest/gtest.h"
#include "geography/mobile.h"
#include "util/headers/int_types.h"

namespace geography {

class ConnectionTest : public testing::Test {
 protected:
  void SetUp() override {
    proto::Area area;
    area.set_id(1);
    a_end_ = Area::FromProto(area);
    area.set_id(2);
    z_end_ = Area::FromProto(area);
  }

  proto::Connection proto_;
  std::unique_ptr<Area> a_end_;
  std::unique_ptr<Area> z_end_;
};

TEST_F(ConnectionTest, TestFromProto) {
  proto_.Clear();
  proto_.set_id(1);
  proto_.set_z(z_end_->id());
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  auto connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(a_end_->id());
  proto_.set_z(z_end_->id());
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_id(1);
  proto_.set_a(a_end_->id());
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_id(1);
  proto_.set_a(a_end_->id());
  proto_.set_z(z_end_->id());
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_id(1);
  proto_.set_a(a_end_->id());
  proto_.set_z(z_end_->id());
  proto_.set_distance_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_id(1);
  proto_.set_a(a_end_->id());
  proto_.set_z(1);
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_id(1);
  proto_.set_a(a_end_->id());
  proto_.set_z(z_end_->id());
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_FALSE(connection.get() == NULL);
  EXPECT_EQ(connection->a(), a_end_.get());
  EXPECT_EQ(connection->z(), z_end_.get());

  auto duplicate = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, duplicate.get());

  const auto& lookup_a = Connection::ByEndpoint(a_end_->id());
  EXPECT_EQ(1, lookup_a.size());
  EXPECT_EQ(connection.get(), *lookup_a.begin());

  const auto& lookup_z = Connection::ByEndpoint(z_end_->id());
  EXPECT_EQ(1, lookup_z.size());
  EXPECT_EQ(connection.get(), *lookup_z.begin());

  const auto* lookup_id = Connection::ById(1);
  EXPECT_EQ(connection.get(), lookup_id);

  const auto& lookup_a_z = Connection::ByEndpoints(a_end_->id(), z_end_->id());
  EXPECT_EQ(1, lookup_a_z.size());
  EXPECT_EQ(connection.get(), *lookup_a_z.begin());
  const auto& lookup_z_a = Connection::ByEndpoints(z_end_->id(), a_end_->id());
  EXPECT_EQ(1, lookup_z_a.size());
  EXPECT_EQ(connection.get(), *lookup_z_a.begin());

  EXPECT_EQ(a_end_->id(), connection->OtherSide(z_end_->id()));
  EXPECT_EQ(z_end_->id(), connection->OtherSide(a_end_->id()));
  EXPECT_EQ(0, connection->OtherSide(10));

  EXPECT_EQ(a_end_.get(), connection->OtherSide(z_end_.get()));
  EXPECT_EQ(z_end_.get(), connection->OtherSide(a_end_.get()));
}

TEST_F(ConnectionTest, TestTraversing) {
  proto_.set_id(1);
  proto_.set_a(a_end_->id());
  proto_.set_z(z_end_->id());
  proto_.set_distance_u(2);
  proto_.set_width_u(1);
  auto connection = Connection::FromProto(proto_);
  util::proto::ObjectId unit1;
  unit1.set_type(1);
  unit1.set_number(1);

  int attempts = 0;
  connection->Register(unit1,
                       [&attempts](const Mobile&) -> Connection::Detection {
                         attempts++;
                         return Connection::Detection();
                       });

  class TestMobile : public Mobile {
  public:
    uint64 speed_u(geography::proto::ConnectionType type) const override {
      return 1;
    }
    const proto::Location& location() const override { return location_; }
    proto::Location* mutable_location() override { return &location_; }
   private:
    proto::Location location_;
  };

  TestMobile mobile;
  proto::Location location;
  location.set_source_area_id(a_end_->id());
  location.set_destination_area_id(z_end_->id());
  location.set_connection_id(connection->ID());
  const DefaultTraverser traverser;
  traverser.Traverse(mobile, &location);
  EXPECT_EQ(1, attempts);
  EXPECT_EQ(a_end_->id(), location.source_area_id());
  EXPECT_EQ(z_end_->id(), location.destination_area_id());
  EXPECT_EQ(1, location.progress_u());

  connection->UnRegister(unit1);
  EXPECT_TRUE(traverser.Traverse(mobile, &location));
  EXPECT_EQ(1, attempts);
  EXPECT_EQ(z_end_->id(), location.source_area_id());
  EXPECT_FALSE(location.has_destination_area_id());
  EXPECT_EQ(0, location.progress_u());
}

} // namespace geography
