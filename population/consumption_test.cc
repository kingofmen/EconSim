// Tests for consumption math.
#include "population/consumption.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "market/goods_utils.h"
#include "population/proto/consumption.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"

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
  Log::SetVerbosity("population/consumption.cc", 10);
  Log::Register(Log::coutLogger);

  market::Add(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  EXPECT_OK(Validate(subs));
  EXPECT_OK(Optimum(subs, prices, &result));
  EXPECT_EQ(3*micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));

  // Initially test symmetric case.
  result.Clear();
  market::Add(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  EXPECT_OK(Validate(subs));
  EXPECT_OK(Optimum(subs, prices, &result));
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kOranges));

  // More extreme symmetric case.
  market::Add(kApples, 7*micro::kOneInU, subs.mutable_consumed());
  market::Add(kOranges, 7*micro::kOneInU, subs.mutable_consumed());
  EXPECT_OK(Validate(subs));
  EXPECT_OK(Optimum(subs, prices, &result));
  EXPECT_EQ(3333333, market::GetAmount(result, kApples));
  EXPECT_EQ(3333333, market::GetAmount(result, kOranges));

  // Asymmetric price.
  market::SetAmount(kApples, 2*micro::kOneInU, &prices);
  market::SetAmount(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  EXPECT_OK(Optimum(subs, prices, &result));
  // Respectively sqrt{2} - 1 and 2\sqrt{2} - 1.
  EXPECT_EQ(414214, market::GetAmount(result, kApples));
  EXPECT_EQ(1828428, market::GetAmount(result, kOranges));

  // Asymmetric crossing points.
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kOranges, 1*micro::kOneInU, subs.mutable_consumed());
  EXPECT_OK(Validate(subs));
  EXPECT_OK(Optimum(subs, prices, &result));
  // -1+2sqrt{1/3}.
  EXPECT_EQ(154700, market::GetAmount(result, kApples));
  // (-1 + 2\sqrt{3})/3.
  EXPECT_EQ(821367, market::GetAmount(result, kOranges));

  // Three goods.
  market::SetAmount(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  market::Add(kBananas, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  market::SetAmount(kBananas, micro::kOneInU, &prices);
  EXPECT_OK(Validate(subs));
  EXPECT_OK(Optimum(subs, prices, &result));
  // Three sevenths.
  EXPECT_EQ(428571, market::GetAmount(result, kApples));
  EXPECT_EQ(428571, market::GetAmount(result, kOranges));
  EXPECT_EQ(428571, market::GetAmount(result, kBananas));

  // Asymmetric prices.
  market::SetAmount(kOranges, 3*micro::kOneInU, &prices);
  EXPECT_OK(Validate(subs));
  EXPECT_OK(Optimum(subs, prices, &result));
  // Cube-root of either 3, or one-ninth; minus one-half, divide by
  // seven-sixths.
  EXPECT_EQ(807643, market::GetAmount(result, kApples));
  EXPECT_EQ(-16500, market::GetAmount(result, kOranges));
  EXPECT_EQ(807643, market::GetAmount(result, kBananas));  

  // Asymmetric crossing points.
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  market::SetAmount(kOranges, 1*micro::kOneInU, subs.mutable_consumed());
  EXPECT_OK(Validate(subs));
  EXPECT_OK(Optimum(subs, prices, &result));
  // One coefficient is 7/2 instead of 7/6, otherwise the same.
  EXPECT_EQ(165738, market::GetAmount(result, kApples));
  EXPECT_EQ(451452, market::GetAmount(result, kOranges));
  EXPECT_EQ(165738, market::GetAmount(result, kBananas));  
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
  EXPECT_THAT(status.error_message(),
              testing::HasSubstr("consumed): Nonpositive coefficient ratio"));
  subs.set_offset_u(3*micro::kOneInU/2);
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
