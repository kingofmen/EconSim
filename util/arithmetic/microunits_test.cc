#include "util/arithmetic/microunits.h"

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
