#include "games/geography/connection.h"

#include <memory>

#include "games/geography/geography.h"
#include "games/geography/mobile.h"
#include "games/geography/proto/geography.pb.h"
#include "gtest/gtest.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.h"

namespace geography {

class ConnectionTest : public testing::Test {
protected:
  void SetUp() override {
    proto::Area area;
    area.mutable_area_id()->set_number(1);
    a_end_ = Area::FromProto(area);
    area.mutable_area_id()->set_number(2);
    z_end_ = Area::FromProto(area);
  }

  proto::Connection proto_;
  std::unique_ptr<Area> a_end_;
  std::unique_ptr<Area> z_end_;
};

TEST_F(ConnectionTest, TestFromProto) {
  const auto connection_id = util::objectid::New("connection", 1);

  proto_.Clear();
  *proto_.mutable_connection_id() = connection_id;
  *proto_.mutable_z_area_id() = z_end_->area_id();
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  auto connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  *proto_.mutable_a_area_id() = a_end_->area_id();
  *proto_.mutable_z_area_id() = z_end_->area_id();
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  *proto_.mutable_connection_id() = connection_id;
  *proto_.mutable_a_area_id() = a_end_->area_id();
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  *proto_.mutable_connection_id() = connection_id;
  *proto_.mutable_a_area_id() = a_end_->area_id();
  *proto_.mutable_z_area_id() = z_end_->area_id();
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  *proto_.mutable_connection_id() = connection_id;
  *proto_.mutable_a_area_id() = a_end_->area_id();
  *proto_.mutable_z_area_id() = z_end_->area_id();
  proto_.set_distance_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  *proto_.mutable_connection_id() = connection_id;
  *proto_.mutable_a_area_id() = a_end_->area_id();
  proto_.mutable_z_area_id()->set_number(1);
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  *proto_.mutable_connection_id() = connection_id;
  *proto_.mutable_a_area_id() = a_end_->area_id();
  *proto_.mutable_z_area_id() = z_end_->area_id();
  proto_.set_distance_u(1);
  proto_.set_width_u(1);
  connection = Connection::FromProto(proto_);
  EXPECT_FALSE(connection.get() == NULL);
  EXPECT_EQ(connection->a(), a_end_.get());
  EXPECT_EQ(connection->z(), z_end_.get());

  auto duplicate = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, duplicate.get());

  const auto& lookup_a = Connection::ByEndpoint(a_end_->area_id());
  EXPECT_EQ(1, lookup_a.size());
  EXPECT_EQ(connection.get(), *lookup_a.begin());

  const auto& lookup_z = Connection::ByEndpoint(z_end_->area_id());
  EXPECT_EQ(1, lookup_z.size());
  EXPECT_EQ(connection.get(), *lookup_z.begin());

  const auto* lookup_id = Connection::ById(connection_id);
  EXPECT_EQ(connection.get(), lookup_id);

  const auto& lookup_a_z =
      Connection::ByEndpoints(a_end_->area_id(), z_end_->area_id());
  EXPECT_EQ(1, lookup_a_z.size());
  EXPECT_EQ(connection.get(), *lookup_a_z.begin());
  const auto& lookup_z_a =
      Connection::ByEndpoints(z_end_->area_id(), a_end_->area_id());
  EXPECT_EQ(1, lookup_z_a.size());
  EXPECT_EQ(connection.get(), *lookup_z_a.begin());

  EXPECT_TRUE(util::objectid::Equal(a_end_->area_id(),
                                    connection->OtherSide(z_end_->area_id())));
  EXPECT_TRUE(util::objectid::Equal(z_end_->area_id(),
                                    connection->OtherSide(a_end_->area_id())));
  util::proto::ObjectId dummy;
  util::proto::ObjectId area_fifty_one;
  area_fifty_one.set_number(51);
  area_fifty_one.set_kind("area");
  EXPECT_TRUE(
      util::objectid::Equal(dummy, connection->OtherSide(area_fifty_one)));

  EXPECT_EQ(a_end_.get(), connection->OtherSide(z_end_.get()));
  EXPECT_EQ(z_end_.get(), connection->OtherSide(a_end_.get()));
}

} // namespace geography
