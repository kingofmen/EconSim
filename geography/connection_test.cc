#include "geography/connection.h"

#include <memory>

#include "geography/geography.h"
#include "geography/proto/geography.pb.h"
#include "gtest/gtest.h"

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
  proto_.set_z(z_end_->id());
  proto_.set_distance(1);
  proto_.set_width(1);
  auto connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(a_end_->id());
  proto_.set_distance(1);
  proto_.set_width(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(a_end_->id());
  proto_.set_z(z_end_->id());
  proto_.set_width(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(a_end_->id());
  proto_.set_z(z_end_->id());
  proto_.set_distance(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(a_end_->id());
  proto_.set_z(1);
  proto_.set_distance(1);
  proto_.set_width(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(a_end_->id());
  proto_.set_z(z_end_->id());
  proto_.set_distance(1);
  proto_.set_width(1);
  connection = Connection::FromProto(proto_);
  EXPECT_FALSE(connection.get() == NULL);
  EXPECT_EQ(connection->a(), a_end_.get());
  EXPECT_EQ(connection->z(), z_end_.get());

  const auto& lookup_a = Connection::ByEndpoint(a_end_->id());
  EXPECT_EQ(1, lookup_a.size());
  EXPECT_EQ(connection.get(), *lookup_a.begin());

  const auto& lookup_z = Connection::ByEndpoint(z_end_->id());
  EXPECT_EQ(1, lookup_z.size());
  EXPECT_EQ(connection.get(), *lookup_z.begin());

  EXPECT_EQ(a_end_->id(), connection->OtherSide(z_end_->id()));
  EXPECT_EQ(z_end_->id(), connection->OtherSide(a_end_->id()));
  EXPECT_EQ(0, connection->OtherSide(10));

  EXPECT_EQ(a_end_.get(), connection->OtherSide(z_end_.get()));
  EXPECT_EQ(z_end_.get(), connection->OtherSide(a_end_.get()));
}


} // namespace geography
