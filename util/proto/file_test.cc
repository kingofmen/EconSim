// Tests for proto file-utility library.

#include "util/proto/file.h"

#include "absl/strings/str_join.h"
#include "gtest/gtest.h"
#include "industry/proto/industry.pb.h"
#include "util/status/status.h"

namespace util {
namespace proto {

const std::string kTestDataLocation = "util/proto/test_data";
  // This is a workaround for Bazel issues 4102 and 4292. When they are
  // fixed, use TEST_SRCDIR/TEST_WORKSPACE instead.
  const std::string kTestDir = "C:/Users/Rolf/base";

TEST(ProtoUtils, TestParseProtoFile) {
  industry::proto::Production proto;
  const std::string filename = "industry.pb.txt";
  auto status = ParseProtoFile(
      absl::StrJoin({kTestDir, kTestDataLocation, filename}, "/"), &proto);
  EXPECT_OK(status) << status.error_message();
  EXPECT_EQ(industry::proto::LT_FIELDS, proto.land_type())
      << proto.DebugString();
}

TEST(ProtoUtils, TestMergeProtoFile) {
  industry::proto::Production proto;
  const std::string filename = "industry.pb.txt";
  auto status = MergeProtoFile(
      absl::StrJoin({kTestDir, kTestDataLocation, filename}, "/"), &proto);
  EXPECT_OK(status) << status.error_message();
  EXPECT_EQ(industry::proto::LT_FIELDS, proto.land_type())
      << proto.DebugString();
  EXPECT_EQ(1, proto.steps_size()) << proto.DebugString();
  status = MergeProtoFile(
      absl::StrJoin({kTestDir, kTestDataLocation, filename}, "/"), &proto);
  EXPECT_OK(status) << status.error_message();
  EXPECT_EQ(industry::proto::LT_FIELDS, proto.land_type())
      << proto.DebugString();
  EXPECT_EQ(2, proto.steps_size()) << proto.DebugString();
}

}  // namespace proto
}  // namespace util
