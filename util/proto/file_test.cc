// Tests for proto file-utility library.

#include "util/proto/file.h"

#include "absl/strings/str_join.h"
#include "gtest/gtest.h"
#include "util/status/status.h"
#include "util/proto/object_id.pb.h"

namespace util {
namespace proto {

const std::string kTestDir = std::getenv("TEST_SRCDIR");
const std::string kWorkdir = std::getenv("TEST_WORKSPACE");
const std::string kTestDataLocation = "util/proto/test_data";

TEST(ProtoUtils, TestParseProtoFile) {
  ObjectId proto;
  const std::string filename = "objectid.pb.txt";
  auto status = ParseProtoFile(
      absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, filename}, "/"),
      &proto);
  EXPECT_OK(status) << status.error_message();
  EXPECT_EQ("test", proto.kind()) << proto.DebugString();
  EXPECT_EQ(1, proto.number()) << proto.DebugString();
  EXPECT_EQ("check", proto.tag()) << proto.DebugString();
}

TEST(ProtoUtils, TestMergeProtoFile) {
  ObjectId proto;
  const std::string filename = "objectid.pb.txt";
  auto status = MergeProtoFile(
      absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, filename}, "/"),
      &proto);
  EXPECT_OK(status) << status.error_message();
  EXPECT_EQ("test", proto.kind()) << proto.DebugString();
  EXPECT_EQ(1, proto.number()) << proto.DebugString();
  EXPECT_EQ("check", proto.tag()) << proto.DebugString();
  
  proto.clear_tag();
  status = MergeProtoFile(
      absl::StrJoin({kTestDir, kWorkdir, kTestDataLocation, filename}, "/"),
      &proto);
  EXPECT_OK(status) << status.error_message();
  EXPECT_EQ("test", proto.kind()) << proto.DebugString();
  EXPECT_EQ(1, proto.number()) << proto.DebugString();
  EXPECT_EQ("check", proto.tag()) << proto.DebugString();
}

}  // namespace proto
}  // namespace util
