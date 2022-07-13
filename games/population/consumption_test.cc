// Tests for consumption math.
#include "games/population/consumption.h"

#include <functional>

#include "games/market/goods_utils.h"
#include "games/population/proto/consumption.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/arithmetic/microunits.h"
#include "util/logging/logging.h"

namespace consumption {
namespace {
constexpr char kApples[] = "apples";
constexpr char kOranges[] = "oranges";
constexpr char kBananas[] = "bananas";

void VerbLog(int level) {
  Log::SetVerbosity("population/consumption.cc", level);
  static bool registered = false;
  if (!registered) {
    Log::Register(Log::coutLogger);
    registered = true;
  }
}

} // namespace

TEST(ConsumptionTest, Optimum) {
  proto::Substitutes subs;
  market::proto::Container result;
  market::proto::Container prices;
  VerbLog(10);

  market::Add(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Optimum(subs, prices, &result).ok());
  // Some roundoff error from calculating the coefficient.
  EXPECT_EQ(3*micro::kOneInU + 12, market::GetAmount(result, kApples));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));

  // Initially test symmetric case.
  result.Clear();
  market::Add(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Optimum(subs, prices, &result).ok());
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kOranges));

  // More extreme symmetric case.
  market::Add(kApples, 7*micro::kOneInU, subs.mutable_consumed());
  market::Add(kOranges, 7*micro::kOneInU, subs.mutable_consumed());
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Optimum(subs, prices, &result).ok());
  EXPECT_EQ(3333333, market::GetAmount(result, kApples));
  EXPECT_EQ(3333333, market::GetAmount(result, kOranges));

  // Asymmetric price.
  market::SetAmount(kApples, 2*micro::kOneInU, &prices);
  market::SetAmount(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  EXPECT_TRUE(Optimum(subs, prices, &result).ok());
  // Respectively sqrt{2} - 1 and 2\sqrt{2} - 1.
  EXPECT_EQ(414214, market::GetAmount(result, kApples));
  EXPECT_EQ(1828428, market::GetAmount(result, kOranges));

  // Asymmetric crossing points.
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kOranges, 1*micro::kOneInU, subs.mutable_consumed());
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Optimum(subs, prices, &result).ok());
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
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Optimum(subs, prices, &result).ok());
  // Three sevenths.
  EXPECT_EQ(428571, market::GetAmount(result, kApples));
  EXPECT_EQ(428571, market::GetAmount(result, kOranges));
  EXPECT_EQ(428571, market::GetAmount(result, kBananas));

  // Asymmetric prices.
  market::SetAmount(kOranges, 3*micro::kOneInU, &prices);
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Optimum(subs, prices, &result).ok());
  // Cube-root of either 3, or one-ninth; minus one-half, divide by
  // seven-sixths. This gives a negative number for oranges, which
  // the algorithm will clamp; the other two then come out to
  // six-sevenths of (sqrt(2) - 1/2).
  EXPECT_EQ(783612, market::GetAmount(result, kApples));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));
  EXPECT_EQ(783612, market::GetAmount(result, kBananas));

  // Asymmetric crossing points.
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  market::SetAmount(kOranges, 1*micro::kOneInU, subs.mutable_consumed());
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Optimum(subs, prices, &result).ok());
  // One coefficient is 7/2 instead of 7/6, otherwise the same.
  EXPECT_EQ(165738, market::GetAmount(result, kApples));
  EXPECT_EQ(451452, market::GetAmount(result, kOranges));
  EXPECT_EQ(165738, market::GetAmount(result, kBananas));  
}

TEST(ConsumptionTest, Validation) {
  proto::Substitutes subs;
  subs.set_name("Test");
  auto status = Validate(subs);
  EXPECT_THAT(status.ToString(),
              testing::HasSubstr("Must specify at least one good"));

  market::Add(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  status = Validate(subs);
  EXPECT_TRUE(status.ok()) << status.message();

  subs.set_offset_u(micro::kOneInU);
  status = Validate(subs);
  EXPECT_THAT(status.ToString(),
              testing::HasSubstr("consumables: D^2 must be strictly greater"));

  market::Add(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  subs.set_min_amount_square_u(4*micro::kOneInU);
  subs.set_offset_u(2*micro::kOneInU);
  status = Validate(subs);
  EXPECT_THAT(status.ToString(),
              testing::HasSubstr("consumables: D^2 must be strictly greater"));

  subs.set_offset_u(2*micro::kOneInU-1);
  status = Validate(subs);
  EXPECT_THAT(status.ToString(),
              testing::HasSubstr("consumed): Nonpositive coefficient ratio"));
  subs.set_offset_u(3*micro::kOneInU/2);
  status = Validate(subs);
  EXPECT_TRUE(status.ok()) << status.message();

  market::Add(kBananas, 3*micro::kOneInU, subs.mutable_consumed());
  subs.set_min_amount_square_u(8*micro::kOneInU);
  subs.set_offset_u(2*micro::kOneInU);
  status = Validate(subs);
  EXPECT_THAT(status.ToString(),
              testing::HasSubstr("consumables: D^2 must be strictly greater"));

  subs.set_offset_u(2*micro::kOneInU-1);
  status = Validate(subs);
  EXPECT_TRUE(status.ok()) << status.message();

  market::Add("pears", 3*micro::kOneInU, subs.mutable_consumed());
  status = Validate(subs);
  EXPECT_THAT(status.ToString(), testing::HasSubstr("handle at most"));
  EXPECT_THAT(status.ToString(), testing::HasSubstr("found 4"));
}

struct FakeMarket : public market::AvailabilityEstimator {
  micro::Measure Available(const std::string& name, int ahead) const override {
    if (available.find(name) == available.end()) {
      return 0;
    }
    return available.at(name);
  }

  bool Available(const market::proto::Container& basket,
                 int ahead) const override {
    for (const auto& good : basket.quantities()) {
      if (Available(good.first, ahead) < good.second) {
        return false;
      }
    }
    return true;
  }

  void Set(const std::string name, micro::Measure amount) {
    available[name] = amount;
  }

  std::unordered_map<std::string, micro::Measure> available;
};

TEST(ConsumptionTest, Consumption) {
  proto::Substitutes subs;
  subs.set_name("Consumption");
  market::proto::Container result;
  market::proto::Container prices;
  VerbLog(3);

  FakeMarket available;
  market::SetAmount(kApples, 0, &prices);
  auto status = Consumption(subs, prices, available, &result);
  EXPECT_THAT(status.ToString(), testing::HasSubstr("Prices must be positive"));
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  market::SetAmount(kBananas, micro::kOneInU, &prices);
  status = Consumption(subs, prices, available, &result);
  EXPECT_THAT(status.ToString(), testing::HasSubstr("Literally no goods"));
  available.Set(kApples, micro::kTenInU);

  market::Add(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  EXPECT_TRUE(Validate(subs).ok());
  status = Consumption(subs, prices, available, &result);
  EXPECT_TRUE(status.ok()) << status.message();
  EXPECT_EQ(3*micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));

  available.Set(kApples, micro::kOneInU);
  status = Consumption(subs, prices, available, &result);
  EXPECT_THAT(status.ToString(), testing::HasSubstr("enough goods available"));

  market::Add(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  available.Set(kOranges, micro::kTenInU);
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Consumption(subs, prices, available, &result).ok());
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kOranges));

  available.Set(kApples, micro::kHalfInU);
  EXPECT_TRUE(Consumption(subs, prices, available, &result).ok());
  EXPECT_EQ(micro::kHalfInU, market::GetAmount(result, kApples));
  EXPECT_EQ(1666666, market::GetAmount(result, kOranges));

  available.Set(kApples, micro::kOneInU);
  available.Set(kOranges, micro::kOneInU);
  available.Set(kBananas, micro::kOneInU);
  market::SetAmount(kApples, 7*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kOranges, 7*micro::kOneInU, subs.mutable_consumed());
  market::SetAmount(kBananas, 7*micro::kOneInU, subs.mutable_consumed());
  EXPECT_TRUE(Validate(subs).ok());
  EXPECT_TRUE(Consumption(subs, prices, available, &result).ok());
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kOranges));
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kBananas));

  available.Set(kApples, micro::kTenInU);
  available.Set(kOranges, micro::kTenInU);
  available.Set(kBananas, 0);
  EXPECT_TRUE(Consumption(subs, prices, available, &result).ok());
  EXPECT_EQ(1828428, market::GetAmount(result, kApples));
  EXPECT_EQ(1828428, market::GetAmount(result, kOranges));
  EXPECT_EQ(0, market::GetAmount(result, kBananas));

  available.Set(kOranges, 0);
  available.Set(kBananas, micro::kTenInU);
  EXPECT_TRUE(Consumption(subs, prices, available, &result).ok());
  EXPECT_EQ(1828428, market::GetAmount(result, kApples));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));
  EXPECT_EQ(1828428, market::GetAmount(result, kBananas));

  available.Set(kBananas, 0);
  EXPECT_TRUE(Consumption(subs, prices, available, &result).ok());
  EXPECT_EQ(7 * micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));
  EXPECT_EQ(0, market::GetAmount(result, kBananas));
}

TEST(ConsumptionTest, GreedyLocal) {
  proto::Substitutes subs;
  subs.set_name("GreedyLocal");
  market::Add(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  market::Add(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  market::proto::Container result;
  market::proto::Container prices;
  // If prices are not set optimum calculations will fail and skipcount, below,
  // will be off.
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  market::SetAmount(kBananas, micro::kOneInU, &prices);
  VerbLog(3);

  struct GreedyLocalMarket : public market::AvailabilityEstimator {
    micro::Measure Available(const std::string& name, int ahead) const override {
      if (available.find(name) == available.end()) {
        return 0;
      }
      if (skipfunc(name)) {
        return 0;
      }

      return available.at(name);
    }

    bool Available(const market::proto::Container& basket,
                   int ahead) const override {
      // Only the initial optimum result uses this method, and we don't want
      // that to succeed.
      return false;
    }

    void Set(const std::string& name, micro::Measure amount) {
      available[name] = amount;
    }
    void SetCustomFunction(std::function<bool(const std::string&)> f) {
      skipfunc = f;
    }

    std::unordered_map<std::string, micro::Measure> available;
    std::function<bool(const std::string&)> skipfunc;
  };
  GreedyLocalMarket greedy;
  std::unordered_map<std::string, int> count;
  int countToSkip = 3;
  auto counter = [&count, &countToSkip](const std::string& n) -> bool {
    // Custom to make things available for sanity check and for
    // greedy-local, but not available for constrained-optimum.
    count[n]++;
    return count[n] == countToSkip;
  };
  greedy.SetCustomFunction(counter);

  greedy.Set(kApples, 2 * micro::kOneInU);
  greedy.Set(kOranges, 0);
  greedy.Set(kBananas, 0);
  auto status = Consumption(subs, prices, greedy, &result);
  EXPECT_THAT(status.ToString(),
              testing::HasSubstr("Not enough goods available"));
  count = {};

  greedy.Set(kApples, 3 * micro::kOneInU);
  status = Consumption(subs, prices, greedy, &result);
  EXPECT_TRUE(status.ok()) << status.message();
  EXPECT_EQ(market::GetAmount(subs.consumed(), kApples),
            market::GetAmount(result, kApples));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));
  count = {};

  // As there are now two goods, the first greedy-local won't fire, so change
  // the skipcount to account for that.
  countToSkip = 2;
  greedy.Set(kApples, 2 * micro::kOneInU);
  greedy.Set(kOranges, 2 * micro::kOneInU);
  status = Consumption(subs, prices, greedy, &result);
  EXPECT_TRUE(status.ok()) << status.message();
  // Some roundoff error here.
  EXPECT_EQ(micro::kOneThirdInU - 1, market::GetAmount(result, kOranges));
  EXPECT_EQ(2 * micro::kOneInU, market::GetAmount(result, kApples));
  count = {};
  
  market::Add(kBananas, 3*micro::kOneInU, subs.mutable_consumed());
  greedy.Set(kApples, 1 * micro::kOneInU);
  greedy.Set(kOranges, 1 * micro::kOneInU);
  greedy.Set(kBananas, 1 * micro::kOneInU);
  status = Consumption(subs, prices, greedy, &result);
  EXPECT_TRUE(status.ok()) << status.message();
  EXPECT_EQ(micro::kOneInU, market::GetAmount(result, kApples));
  EXPECT_EQ(600000, market::GetAmount(result, kBananas));
  EXPECT_EQ(0, market::GetAmount(result, kOranges));
}

TEST(ConsumptionTest, Minima) {
  proto::Substitutes subs;
  subs.set_name("Minimum");
  market::Add(kApples, 3*micro::kOneInU, subs.mutable_consumed());
  market::Add(kOranges, 3*micro::kOneInU, subs.mutable_consumed());
  market::proto::Container result;
  market::proto::Container prices;
  market::SetAmount(kApples, micro::kOneInU, &prices);
  market::SetAmount(kOranges, micro::kOneInU, &prices);
  market::SetAmount(kBananas, micro::kOneInU, &prices);
  VerbLog(3);

  FakeMarket available;
  available.Set(kOranges, 4*micro::kOneInU);

  auto status = Consumption(subs, prices, available, &result);
  EXPECT_TRUE(status.ok()) << status.ToString();
  EXPECT_EQ(3*micro::kOneInU, market::GetAmount(result, kOranges));

  market::Add(kApples, micro::kHalfInU, subs.mutable_minimum());
  status = Consumption(subs, prices, available, &result);
  EXPECT_THAT(status.ToString(), testing::HasSubstr(": Minimum "));

  available.Set(kApples, 4*micro::kOneInU);
  market::SetAmount(kOranges, micro::kHalfInU, &prices);
  status = Consumption(subs, prices, available, &result);
  EXPECT_TRUE(status.ok()) << status.message();
  EXPECT_EQ(micro::kHalfInU, market::GetAmount(result, kApples));
  // Coefficients are one-half, y comes out to five thirds.
  EXPECT_EQ(1666666, market::GetAmount(result, kOranges));

  // Three goods, minimum 0.5 apples.
  market::Add(kBananas, 3*micro::kOneInU, subs.mutable_consumed());
  available.Set(kBananas, 4*micro::kOneInU);
  status = Consumption(subs, prices, available, &result);
  EXPECT_TRUE(status.ok()) << status.message();
  EXPECT_EQ(micro::kHalfInU, market::GetAmount(result, kApples));

  // Oranges are cheap so expect more of them. Coefficients are seven-sixths;
  // orange amount is root-2 minus one-half, divide seven-sixths. However that
  // is with D^2=1, and in fact it is slightly reduced due to apple constraint,
  // so the number below is correct. Then, banana amount is root one-half minus
  // one-half, divide the same seven-sixths, and again adjust for reduced D^2.
  EXPECT_EQ(736056, market::GetAmount(result, kOranges));
  EXPECT_EQ(153741, market::GetAmount(result, kBananas));
}

} // namespace consumption
