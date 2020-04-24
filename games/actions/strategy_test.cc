#include "games/actions/strategy.h"

#include "games/actions/proto/strategy.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/status/status.h"

TEST(Strategy, TestRegistry) {
  actions::proto::Strategy original;
  actions::proto::Strategy load;

  auto status = actions::RegisterStrategy(original);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("'define' field"));

  original.set_define("test");
  status = actions::RegisterStrategy(original);
  EXPECT_TRUE(status.ok()) << status.error_message();

  status = actions::RegisterStrategy(original);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("already registered"));

  status = actions::LoadStrategy("nonesuch", nullptr);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("null Strategy"));

  status = actions::LoadStrategy("nonesuch", &load);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("not found"));

  status = actions::LoadStrategy("test", &load);
  EXPECT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(original.define(), load.lookup());
}

