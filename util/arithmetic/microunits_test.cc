#include "util/arithmetic/microunits.h"

#include <limits>

#include "gtest/gtest.h"
#include "util/headers/int_types.h"

namespace micro {

namespace {
constexpr char kTestGood1[] = "TestGood1";
constexpr char kTestGood2[] = "TestGood2";
}

TEST(MicroUnitsTest, MultiplyInts) {
  int64 lhs = 1;
  int64 rhs_u = kOneInU;
  EXPECT_EQ(MultiplyU(lhs, rhs_u), 1);
  int64 lhs_u = kOneInU;
  EXPECT_EQ(MultiplyU(lhs_u, rhs_u), kOneInU);
}

TEST(MicroUnitsTest, DivideInts) {
  int64 lhs = 10;
  int64 rhs_u = 2 * kOneInU;
  EXPECT_EQ(DivideU(lhs, rhs_u), 5);
  int64 lhs_u = kTenInU;
  EXPECT_EQ(DivideU(lhs_u, rhs_u), 5 * kOneInU);
}

TEST(MicroUnitsTest, DivideOverflowingInts) {
  int64 bignum = std::numeric_limits<int64>::max();
  int64 smallnum_u = kOneInU;
  uint64 overflow = 0;
  EXPECT_EQ(bignum, DivideU(bignum, smallnum_u, &overflow));
  EXPECT_EQ(0, overflow);

  // Dividing by one-half is equivalent to multiplying by two.
  smallnum_u = kOneInU / 2;
  EXPECT_EQ(bignum << 1, DivideU(bignum, smallnum_u, &overflow));
  // Overflow is into 64th bit.
  EXPECT_EQ(0x8000000000000000u, overflow);

  smallnum_u = kOneInU / 4;
  EXPECT_EQ(bignum << 2, DivideU(bignum, smallnum_u, &overflow));
  // 64th and 65th bits overflow.
  EXPECT_EQ(0x8000000000000001u, overflow);

  smallnum_u = kOneInU / 8;
  EXPECT_EQ(bignum << 3, DivideU(bignum, smallnum_u, &overflow));
  // 64th, 65th, 66th bits overflow.
  EXPECT_EQ(0x8000000000000003u, overflow);

  bignum = std::numeric_limits<int64>::min();
  smallnum_u = kOneInU;
  EXPECT_EQ(std::numeric_limits<int64>::min(),
            DivideU(bignum, smallnum_u, &overflow));
  // No overflow in this case!
  EXPECT_EQ(0, overflow);

  bignum = std::numeric_limits<int64>::min();
  smallnum_u = 1;
  EXPECT_EQ(0,
            DivideU(bignum, smallnum_u, &overflow));
  // Maximum possible overflow - one million into the overflow bits, of which
  // one bit is the MSB of the return value!
  EXPECT_EQ(kOneInU / 2, overflow);

  EXPECT_EQ(0, DivideU(bignum, 0, &overflow));
  // All overflow bits set to indicate badness.
  EXPECT_EQ(-1, overflow);
  
}

TEST(MicroUnitsTest, SquareRoot) {
  EXPECT_EQ(SqrtU(kOneInU), kOneInU);
  EXPECT_EQ(SqrtU(kHundredInU), kTenInU);
}

TEST(MicroUnitsTest, Sanity) {
  EXPECT_EQ(kOneFourthInU, MultiplyU(kHalfInU, kHalfInU));
  EXPECT_EQ(kThreeFourthsInU, MultiplyU(kThreeFourthsInU, kOneInU));
  EXPECT_EQ(kTenInU, MultiplyU(kHundredInU, kOneTenthInU));
}

TEST(MicroUnitsTest, DisplayString) {
  EXPECT_EQ("1.00", DisplayString(kOneInU, 2));
  EXPECT_EQ("1.000000", DisplayString(kOneInU, 8));
  EXPECT_EQ("0.1", DisplayString(kOneTenthInU, 1));
  EXPECT_EQ("0.25", DisplayString(kOneFourthInU, 2));
  EXPECT_EQ("0.2", DisplayString(kOneFourthInU, 1));
  EXPECT_EQ("10.25", DisplayString(kTenInU + kOneFourthInU, 2));
  EXPECT_EQ("1.234567", DisplayString(1234567, 6));
}

TEST(MicroUnitsTest, NthRoot) {
  EXPECT_EQ(kOneInU, NRootU(0, 317928));
  EXPECT_EQ(kOneInU, NRootU(1, kOneInU));
  EXPECT_EQ(kTenInU, NRootU(1, kTenInU));
  EXPECT_EQ(kOneInU, NRootU(-1, kOneInU));
  EXPECT_EQ(kOneTenthInU, NRootU(-1, kTenInU));

  EXPECT_EQ(3 * kOneInU, NRootU(3, 27 * kOneInU));
  EXPECT_EQ(2100000, NRootU(3, 9261000));
  EXPECT_EQ(333333, NRootU(-3, 27 * kOneInU));
}

TEST(MicroUnitsTest, Power) {
  EXPECT_EQ(0, PowU(0, 10));
  EXPECT_EQ(kOneInU, PowU(0, 0));
  EXPECT_EQ(kOneInU, PowU(kTenInU, 0));
  EXPECT_EQ(kTenInU, PowU(kTenInU, 1));
  EXPECT_EQ(kOneTenthInU, PowU(kTenInU, -1));
  EXPECT_EQ(kOneHundredthInU, PowU(kTenInU, -2));

  EXPECT_EQ(kHundredInU, PowU(kTenInU, 2));
  EXPECT_EQ(kThousandInU, PowU(kTenInU, 3));
  EXPECT_EQ(kOneFourthInU, PowU(kHalfInU, 2));
}

}  // namespace micro
