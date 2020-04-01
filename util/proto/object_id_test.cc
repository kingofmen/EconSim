#include "util//proto/object_id.h"

#include "absl/strings/str_join.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace util {
namespace objectid {

TEST(ObjectId, TestTags) {
  util::proto::ObjectId obj_id;
  auto status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("without type or kind"));

  obj_id.set_type(1);
  status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("no number or tag"));

  obj_id.set_number(1);
  status = Canonicalise(&obj_id);
  EXPECT_OK(status) << status.error_message();

  obj_id.set_tag("one");
  status = Canonicalise(&obj_id);
  EXPECT_OK(status) << status.error_message();
  EXPECT_FALSE(obj_id.has_tag());
  EXPECT_EQ(1, obj_id.number());

  obj_id.set_tag("one");
  status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("(number) already exists"));

  obj_id.set_number(2);
  status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("(tag) already exists"));

  obj_id.clear_number();
  status = Canonicalise(&obj_id);
  EXPECT_OK(status) << status.error_message();
  EXPECT_FALSE(obj_id.has_tag());
  EXPECT_EQ(1, obj_id.number());

  obj_id.set_tag("two");
  obj_id.clear_number();
  status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("Cannot find number"));

  obj_id.clear_tag();
  obj_id.set_number(1);
  EXPECT_EQ(Tag(obj_id), "one");
}

TEST(ObjectId, TestTagsWithKind) {
  util::proto::ObjectId obj_id;
  auto status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("without type or kind"));

  obj_id.set_kind("type_one");
  status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("no number or tag"));

  obj_id.set_number(1);
  status = Canonicalise(&obj_id);
  EXPECT_OK(status) << status.error_message();

  obj_id.set_tag("one");
  status = Canonicalise(&obj_id);
  EXPECT_OK(status) << status.error_message();
  EXPECT_FALSE(obj_id.has_tag());
  EXPECT_EQ(1, obj_id.number());

  obj_id.set_tag("one");
  status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("(number) already exists"));

  obj_id.set_number(2);
  status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("(tag) already exists"));

  obj_id.clear_number();
  status = Canonicalise(&obj_id);
  EXPECT_OK(status) << status.error_message();
  EXPECT_FALSE(obj_id.has_tag());
  EXPECT_EQ(1, obj_id.number());

  obj_id.set_tag("two");
  obj_id.clear_number();
  status = Canonicalise(&obj_id);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("Cannot find number"));

  obj_id.clear_tag();
  obj_id.set_number(1);
  EXPECT_EQ(Tag(obj_id), "one");
}


}  // namespace objectid
}  // namespace util
