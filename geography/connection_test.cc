#include "geography/connection.h"

#include <memory>

#include "geography/proto/geography.pb.h"
#include "gtest/gtest.h"

namespace geography {

class ConnectionTest : public testing::Test {
 protected:
  void SetUp() override {}

  proto::Connection proto_;
};

TEST_F(ConnectionTest, TestFromProto) {
  proto_.Clear();
  proto_.set_z(1);
  proto_.set_distance(1);
  proto_.set_width(1);
  auto connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(1);
  proto_.set_distance(1);
  proto_.set_width(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(1);
  proto_.set_z(1);
  proto_.set_width(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(1);
  proto_.set_z(1);
  proto_.set_distance(1);
  connection = Connection::FromProto(proto_);
  EXPECT_EQ(NULL, connection.get());

  proto_.Clear();
  proto_.set_a(1);
  proto_.set_z(1);
  proto_.set_distance(1);
  proto_.set_width(1);
  connection = Connection::FromProto(proto_);
  EXPECT_NE(NULL, connection.get());
}


} // namespace geography
