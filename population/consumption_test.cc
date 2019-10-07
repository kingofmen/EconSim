// Tests for consumption math.
#include "population/consumption.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "market/goods_utils.h"
#include "population/proto/consumption.pb.h"
#include "util/arithmetic/microunits.h"

namespace consumption {
namespace {
constexpr char kApples[] = "apples";
constexpr char kOranges[] = "oranges";
constexpr char kBananas[] = "bananas";

} // namespace

TEST(ConsumptionTest, Optimum) {
  proto::Substitutes subs;
  market::proto::Container result;
  market::proto::Container prices;

  market::Add(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  EXPECT_OK(Optimum(subs, prices, &result));
  EXPECT_EQ(3*micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));

  // Initially test symmetric case.
  market::Add(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  EXPECT_OK(Optimum(subs, prices, &result));
}

TEST(ConsumptionTest, Validation) {
  proto::Substitutes subs;
  subs.set_name("Test");
  auto status = Validate(subs);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("Must specify at least one good"));

  market::Add(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  status = Validate(subs);
  EXPECT_OK(status) << status.error_message();

  subs.set_offset_u(micro::kOneInU);
  status = Validate(subs);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("consumables: D^2 must be strictly greater"));

  market::Add(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  subs.set_min_amount_square_u(4*micro::kOneInU);
  subs.set_offset_u(2*micro::kOneInU);
  status = Validate(subs);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("consumables: D^2 must be strictly greater"));

  subs.set_offset_u(2*micro::kOneInU-1);
  status = Validate(subs);
  EXPECT_OK(status) << status.error_message();

  market::Add(kBananas, 3*micro::kOneInU, subs.mutable_consumed());
  subs.set_min_amount_square_u(8*micro::kOneInU);
  subs.set_offset_u(2*micro::kOneInU);
  status = Validate(subs);
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("consumables: D^2 must be strictly greater"));

  subs.set_offset_u(2*micro::kOneInU-1);
  status = Validate(subs);
  EXPECT_OK(status) << status.error_message();

  market::Add("pears", 3*micro::kOneInU, subs.mutable_consumed());
  status = Validate(subs);
  EXPECT_THAT(status.error_message(), testing::HasSubstr("handle at most"));
  EXPECT_THAT(status.error_message(), testing::HasSubstr("found 4"));
}

} // namespace consumption
