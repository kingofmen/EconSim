#include "util/arithmetic/microunits.h"

#include <limits>

#include "gtest/gtest.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
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

TEST(MicroUnitsTest, ScaleContainer) {
  market::proto::Container lhs;
  market::SetAmount(kTestGood1, 1, &lhs);
  market::SetAmount(kTestGood2, 10, &lhs);
  MultiplyU(lhs, kTenInU);
  EXPECT_EQ(market::GetAmount(lhs, kTestGood1), 10);
  EXPECT_EQ(market::GetAmount(lhs, kTestGood2), 100);

  market::proto::Container lhs_u;
  market::SetAmount(kTestGood1, kOneInU, &lhs_u);
  market::SetAmount(kTestGood2, kTenInU, &lhs_u);
  MultiplyU(lhs_u, kTenInU);
  EXPECT_EQ(market::GetAmount(lhs_u, kTestGood1), kTenInU);
  EXPECT_EQ(market::GetAmount(lhs_u, kTestGood2), kHundredInU);
}

TEST(MicroUnitsTest, MatrixVectorMultiplication) {
  market::proto::Container rhs_u;
  market::SetAmount(kTestGood1, kOneInU, &rhs_u);
  market::SetAmount(kTestGood2, kTenInU, &rhs_u);
  
  market::proto::Container lhs;
  market::SetAmount(kTestGood1, 1, &lhs);
  market::SetAmount(kTestGood2, 10, &lhs);
  MultiplyU(lhs, rhs_u);
  EXPECT_EQ(market::GetAmount(lhs, kTestGood1), 1);
  EXPECT_EQ(market::GetAmount(lhs, kTestGood2), 100);

  market::proto::Container lhs_u;
  market::SetAmount(kTestGood1, kOneInU, &lhs_u);
  market::SetAmount(kTestGood2, kTenInU, &lhs_u);
  MultiplyU(lhs_u, rhs_u);
  EXPECT_EQ(market::GetAmount(lhs_u, kTestGood1), kOneInU);
  EXPECT_EQ(market::GetAmount(lhs_u, kTestGood2), kHundredInU);
}

}  // namespace micro
