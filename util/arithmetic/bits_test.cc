#include "util/arithmetic/bits.h"

#include "gtest/gtest.h"

namespace bits {

TEST(BitsTest, MakeMask) {
  auto mask = MakeMask(0);
  EXPECT_EQ(mask, kZero);
  EXPECT_TRUE(mask.none());
  mask = MakeMask(0, 1, 2, 3);
  EXPECT_EQ(mask, kZero);
  mask = MakeMask(1, 1);
  EXPECT_EQ(mask, kOne);
  EXPECT_EQ(mask.count(), 1);
  EXPECT_TRUE(mask.any());
  mask = MakeMask(3, 1, 1, 2);
  EXPECT_EQ(mask, kOne | kTwo);
  mask = MakeMask(3, 1, 1, 2, 5, 6, 7);
  EXPECT_EQ(mask, kOne | kTwo);
  mask = MakeMask(2, 1, 32);
  EXPECT_EQ(mask, kOne | kThirtyTwo);
  mask = MakeMask(32, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32);
  EXPECT_EQ(mask, kAll);
  EXPECT_TRUE(mask.all());
  EXPECT_EQ(mask.count(), 32);
}

} // namespace bits
