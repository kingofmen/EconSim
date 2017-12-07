// Tests for population units.
#include "population/popunit.h"

#include "geography/proto/geography.pb.h"
#include "industry/industry.h"
#include "industry/proto/industry.pb.h"
#include "market/market.h"
#include "population/proto/population.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/keywords/keywords.h"
#include "gtest/gtest.h"

namespace population {
namespace {
constexpr char kTestCulture1[] = "Norwegian";
constexpr char kTestCulture2[] = "Scots";
} // namespace

class PopulationTest : public testing::Test {
protected:
  void SetUp() override {
    pop_.Proto()->add_males(1);
    market::proto::Quantity culture;
    culture.set_kind(kTestCulture1);
    culture += micro::kOneInU;
    *pop_.Proto()->mutable_tags() << culture;

    fish_.set_kind("fish");
    house_.set_kind("house");
    youtube_.set_kind("youtube");
    dinner_.set_kind("dinner");
    grain_.set_kind("grain");

    fish_package_ = level_.add_packages();
    house_package_ = level_.add_packages();
    fish_ += micro::kOneInU;
    *fish_package_->mutable_consumed() << fish_;
    house_ += micro::kOneInU;
    *house_package_->mutable_consumed() << house_;

    dinner_from_grain_proto_ = dinner_from_grain_.Proto();
    dinner_from_fish_proto_ = dinner_from_fish_.Proto();
  }

  void SetupProduction() {
    dinner_from_fish_proto_->set_name("dinner_from_fish");
    dinner_from_fish_proto_->add_scaling_effects_u(micro::kOneInU);
    dinner_from_fish_proto_->add_scaling_effects_u(8 * micro::kOneInU / 10);
    dinner_from_fish_proto_->add_scaling_effects_u(micro::kOneInU / 2);
    auto* step = dinner_from_fish_proto_->add_steps();
    auto* variant1 = step->add_variants();
    auto* variant2 = step->add_variants();
    auto* variant3 = step->add_variants();

    house_ += micro::kOneInU;
    fish_ += micro::kOneInU;
    *variant1->mutable_fixed_capital() << house_;
    *variant1->mutable_consumables() << fish_;
    fish_ += 2 * micro::kOneInU;
    *variant2->mutable_consumables() << fish_;
    youtube_ += micro::kOneInU;
    *variant3->mutable_consumables() << youtube_;
    dinner_ += micro::kOneInU;
    *dinner_from_fish_proto_->mutable_outputs() << dinner_;

    dinner_from_grain_proto_->set_name("dinner_from_grain");
    dinner_from_grain_proto_->add_scaling_effects_u(micro::kOneInU);
    dinner_from_grain_proto_->add_scaling_effects_u(8 * micro::kOneInU / 10);
    step = dinner_from_grain_proto_->add_steps();
    variant1 = step->add_variants();
    variant2 = step->add_variants();

    grain_ += 2 * micro::kOneInU;
    *variant1->mutable_consumables() << grain_;

    grain_ += micro::kOneInU;
    house_ += micro::kOneInU;
    *variant2->mutable_consumables() << grain_;
    *variant2->mutable_fixed_capital() << house_;
    dinner_ += micro::kOneInU;
    *dinner_from_grain_proto_->mutable_outputs() << dinner_;
  }

  void SetupMarket() {
    market_.RegisterGood(fish_.kind());
    market_.RegisterGood(youtube_.kind());
    market_.RegisterGood(dinner_.kind());
    market_.RegisterGood(grain_.kind());
    market_.RegisterGood(house_.kind());
    market_.Proto()->set_credit_limit(micro::kHundredInU);
    market_.Proto()->set_name("market");
    market::SetAmount(fish_.kind(), micro::kOneInU,
                      market_.Proto()->mutable_prices_u());
    market::SetAmount(youtube_.kind(), 2 * micro::kOneInU,
                      market_.Proto()->mutable_prices_u());
    market::SetAmount(dinner_.kind(), 3 * micro::kOneInU,
                      market_.Proto()->mutable_prices_u());
  }

  void SellGoods(market::Measure fish = 0, market::Measure youtube = 0,
                 market::Measure grain = 0) {
    fish_.set_amount(fish);
    youtube_.set_amount(youtube);
    grain_.set_amount(grain);
    seller_ += fish_;
    seller_ += youtube_;
    seller_ += grain_;
    EXPECT_EQ(market_.TryToSell(fish_, &seller_), fish);
    market_.TryToSell(youtube_, &seller_);
    market_.TryToSell(grain_, &seller_);
    EXPECT_GE(market_.AvailableImmediately(fish_.kind()), fish);
  }

  PopUnit pop_;
  proto::ConsumptionLevel level_;
  proto::ConsumptionPackage* fish_package_;
  proto::ConsumptionPackage* house_package_;
  industry::Production dinner_from_fish_;
  industry::proto::Production* dinner_from_fish_proto_;
  industry::Production dinner_from_grain_;
  industry::proto::Production* dinner_from_grain_proto_;
  market::Market market_;
  market::proto::Quantity fish_;
  market::proto::Quantity house_;
  market::proto::Quantity youtube_;
  market::proto::Quantity dinner_;
  market::proto::Quantity grain_;
  market::proto::Container prices_;
  market::proto::Container seller_;
};

TEST_F(PopulationTest, CheapestPackage) {
  SetupMarket();
  const proto::ConsumptionPackage* cheapest = nullptr;
  // Pop has no resources, can afford nothing.
  EXPECT_EQ(pop_.CheapestPackage(level_, market_, cheapest), nullptr);

  fish_ += micro::kOneInU;
  *pop_.mutable_wealth() << fish_;
  // Pop can now eat fish.
  EXPECT_EQ(pop_.CheapestPackage(level_, market_, cheapest), fish_package_);
  EXPECT_EQ(cheapest, fish_package_);

  house_ += micro::kOneInU;
  *pop_.mutable_wealth() << house_;
  fish_ += micro::kOneInU;
  *market_.Proto()->mutable_prices_u() << fish_;
  // Fish is now more expensive than house.
  EXPECT_EQ(pop_.CheapestPackage(level_, market_, cheapest), house_package_);
  EXPECT_EQ(cheapest, house_package_);

  market::proto::Quantity scots;
  scots.set_kind(kTestCulture2);
  scots += micro::kOneInU;
  *house_package_->mutable_required_tags() << scots;
  // House is now forbidden.
  EXPECT_EQ(pop_.CheapestPackage(level_, market_, cheapest), fish_package_);
  EXPECT_EQ(cheapest, fish_package_);

  market::proto::Quantity culture;
  culture.set_kind(kTestCulture1);
  culture += micro::kOneInU;
  *fish_package_->mutable_required_tags() << culture;
  market::Clear(house_package_->mutable_required_tags());
  culture += micro::kOneInU;
  *house_package_->mutable_required_tags() << culture;
  // Both packages are allowed again, so it'll be house.
  EXPECT_EQ(pop_.CheapestPackage(level_, market_, cheapest), house_package_);
  EXPECT_EQ(cheapest, house_package_);

  auto* cheap_package = level_.add_packages();
  youtube_ += 1;
  *cheap_package->mutable_consumed() << youtube_;
  SellGoods(0, 1, 0);

  // YouTube is cheaper, but only available from the market, and the pop has no
  // money. Without credit it won't be able to buy anything.
  market_.Proto()->set_credit_limit(0);
  EXPECT_EQ(pop_.CheapestPackage(level_, market_, cheapest), house_package_);
  // For the first time, the cheapest package differs from the best one.
  EXPECT_EQ(cheapest, cheap_package);

  // With credit, the cheap package is the best.
  market_.Proto()->set_credit_limit(100 * micro::kOneInU);
  EXPECT_EQ(pop_.CheapestPackage(level_, market_, cheapest), cheap_package);
  EXPECT_EQ(cheapest, cheap_package);
}

TEST_F(PopulationTest, Consume) {
  SetupMarket();
  fish_ += market::GetAmount(fish_package_->consumed(), fish_);
  *pop_.mutable_wealth() << fish_;
  // Pop can now eat fish.
  EXPECT_TRUE(pop_.Consume(level_, &market_));
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);

  house_ += micro::kOneInU;
  *pop_.mutable_wealth() << house_;
  fish_ += micro::kOneInU;
  *market_.Proto()->mutable_prices_u() << fish_;
  fish_ += micro::kOneInU;
  *pop_.mutable_wealth() << fish_;
  // Fish is now more expensive than house.
  EXPECT_TRUE(pop_.Consume(level_, &market_));
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), micro::kOneInU);
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), house_), 0);

  market::proto::Quantity tools;
  tools.set_kind("tools");
  tools += micro::kOneInU;
  *house_package_->mutable_capital() << tools;
  house_ += micro::kOneInU;
  *pop_.mutable_wealth() << house_;

  // No tools, eat the fish.
  EXPECT_TRUE(pop_.Consume(level_, &market_));
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), house_), micro::kOneInU);

  // With tools available, use them.
  tools += micro::kOneInU;
  fish_ += micro::kOneInU;
  *pop_.mutable_wealth() << fish_ << tools;
  EXPECT_TRUE(pop_.Consume(level_, &market_));
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), micro::kOneInU);
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), house_), 0);
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), tools), micro::kOneInU);

  // Get rid of the fish.
  EXPECT_TRUE(pop_.Consume(level_, &market_));
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);

  // Put some fish on the market.
  SellGoods(micro::kOneInU, 0, 0);
  // Remove credit, consumption should be impossible.
  market_.Proto()->set_credit_limit(0);
  EXPECT_FALSE(pop_.Consume(level_, &market_));
  EXPECT_EQ(market_.AvailableImmediately(fish_.kind()), micro::kOneInU);
  // Restore credit and the pop can buy fish to eat.
  market_.Proto()->set_credit_limit(100 * market_.GetPriceU(fish_.kind()));
  EXPECT_TRUE(pop_.Consume(level_, &market_));
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);
  EXPECT_EQ(market_.AvailableImmediately(fish_.kind()), 0);
}

TEST_F(PopulationTest, ConsumptionTags) {
  auto& fish_tags = *fish_package_->mutable_tags();
  market::proto::Quantity bad_breath;
  bad_breath.set_kind("halitosis");
  bad_breath += micro::kOneInU;
  fish_tags << bad_breath;

  market::proto::Quantity satiation;
  satiation.set_kind("wellfedness");
  satiation += micro::kOneInU;
  *level_.mutable_tags() << satiation;

  fish_ += micro::kOneInU;
  *pop_.mutable_wealth() << fish_;
  EXPECT_TRUE(pop_.Consume(level_, &market_));
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);
  EXPECT_EQ(market::GetAmount(pop_.Proto()->tags(), bad_breath),
            micro::kOneInU);
  EXPECT_EQ(market::GetAmount(pop_.Proto()->tags(), satiation), micro::kOneInU);
}

TEST_F(PopulationTest, AutoProduction) {
  proto::AutoProduction labour;
  market::proto::Quantity work;
  work.set_kind("labour");
  work += micro::kOneInU;
  *labour.mutable_output() << work;

  proto::AutoProduction prayer;
  market::proto::Quantity words;
  words.set_kind("kneeling");
  words += micro::kOneInU;
  *prayer.mutable_output() << words;

  work += 2 * micro::kOneInU;
  words += micro::kOneInU;
  market_.RegisterGood(work.kind());
  market_.RegisterGood(words.kind());
  market_.Proto()->set_credit_limit(micro::kHundredInU);
  *market_.Proto()->mutable_prices_u() << work;
  *market_.Proto()->mutable_prices_u() << words;

  // Prayer is cheaper, so the pop should choose to work.
  std::vector<const proto::AutoProduction*> production = {&labour, &prayer};
  pop_.AutoProduce(production, &market_);
  EXPECT_EQ(market_.AvailableImmediately(work.kind()), micro::kOneInU);
  EXPECT_EQ(market_.AvailableImmediately(words.kind()), 0);

  market::proto::Quantity culture;
  culture.set_kind(kTestCulture2);
  culture += micro::kOneInU;
  *labour.mutable_required_tags() << culture;

  // Pop can no longer work, so now it should pray.
  pop_.AutoProduce(production, &market_);
  EXPECT_EQ(market_.AvailableImmediately(work.kind()), micro::kOneInU);
  EXPECT_EQ(market_.AvailableImmediately(words.kind()), micro::kOneInU);

  culture += micro::kTenInU;
  *pop_.Proto()->mutable_tags() << culture;
  // With ability to work restored, pop should work again.
  pop_.AutoProduce(production, &market_);
  EXPECT_EQ(market_.AvailableImmediately(work.kind()), 2 * micro::kOneInU);
  EXPECT_EQ(market_.AvailableImmediately(words.kind()), micro::kOneInU);
}

TEST_F(PopulationTest, EndTurn) {
  fish_ += micro::kOneInU;
  *pop_.mutable_wealth() << fish_;
  youtube_ += micro::kOneInU;
  *pop_.mutable_wealth() << youtube_;

  market::proto::Container decay_rates;
  // Fish and guests stink after three days.
  market::SetAmount(fish_, &decay_rates);
  // A good cat video is a thing of joy forever.
  youtube_ += micro::kOneInU;
  market::SetAmount(youtube_, &decay_rates);
  market::SetAmount(kTestCulture1, micro::kOneInU / 2, &decay_rates);

  pop_.EndTurn(decay_rates);
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);
  EXPECT_FALSE(market::Contains(pop_.Proto()->wealth(), fish_));
  EXPECT_EQ(market::GetAmount(pop_.Proto()->wealth(), youtube_),
            micro::kOneInU);
  EXPECT_EQ(market::GetAmount(pop_.Proto()->tags(), kTestCulture1),
            micro::kOneInU / 2);
}

TEST_F(PopulationTest, TryProductionStep) {
  SetupProduction();
  SetupMarket();
  industry::decisions::LocalProfitMaximiser evaluator;
  geography::proto::Field field;

  auto progress = dinner_from_fish_.MakeProgress(3 * micro::kOneInU);
  EXPECT_EQ(28 * micro::kOneInU / 10, dinner_from_fish_.EfficiencyU(progress));
  auto fish_info = evaluator.GetProductionInfo(
      dinner_from_fish_, pop_.Proto()->wealth(), market_, field);

  EXPECT_FALSE(pop_.TryProductionStep(dinner_from_fish_, fish_info, &field,
                                      &progress, &market_))
      << fish_info.DebugString();

  SellGoods(2 * micro::kOneInU);

  // Process makes no profit because it has no output.
  dinner_from_fish_proto_->mutable_outputs()->Clear();
  fish_info = evaluator.GetProductionInfo(
      dinner_from_fish_, pop_.Proto()->wealth(), market_, field);
  EXPECT_FALSE(pop_.TryProductionStep(dinner_from_fish_, fish_info, &field,
                                      &progress, &market_))
      << fish_info.DebugString();

  const std::string kEntertainment("entertainment");
  auto entertainment = market::MakeQuantity(kEntertainment, micro::kTenInU);
  market_.RegisterGood(entertainment.kind());
  market::SetAmount(entertainment, market_.Proto()->mutable_prices_u());
  entertainment.set_amount(micro::kOneInU);
  market::SetAmount(entertainment, dinner_from_fish_proto_->mutable_outputs());
  EXPECT_EQ(dinner_from_fish_.EfficiencyU(progress),
            market::GetAmount(dinner_from_fish_.ExpectedOutput(progress),
                              entertainment));
  fish_info = evaluator.GetProductionInfo(
      dinner_from_fish_, pop_.Proto()->wealth(), market_, field);

  EXPECT_TRUE(pop_.TryProductionStep(dinner_from_fish_, fish_info, &field,
                                     &progress, &market_))
      << fish_info.DebugString();

  // Good will be sold to the market as it is not subsistence.
  EXPECT_EQ(0, market::GetAmount(pop_.wealth(), kEntertainment));
  EXPECT_EQ(dinner_from_fish_.EfficiencyU(progress),
            market_.AvailableImmediately(kEntertainment));
  // Money on hand should be equal to price of output, less price of inputs.
  // Don't count credit.
  EXPECT_EQ(market_.GetPriceU(kEntertainment) - fish_info.total_unit_cost_u(),
            market_.MaxMoney(pop_.wealth()) - market_.MaxCredit(pop_.wealth()))
      << pop_.wealth().DebugString() << "\n"
      << market_.Proto()->DebugString() << "\n"
      << fish_info.DebugString() << "\n"
      << dinner_from_fish_.Proto()->DebugString();
  EXPECT_TRUE(dinner_from_fish_.Complete(progress));

  EXPECT_FALSE(pop_.TryProductionStep(dinner_from_fish_, fish_info, &field,
                                      &progress, &market_))
      << fish_info.DebugString();

  // Make entertainment a subsistence good, and check that it's no longer sold
  // as soon as it is produced.
  progress = dinner_from_fish_.MakeProgress(3 * micro::kOneInU);
  SellGoods(2 * micro::kOneInU);
  level_.Clear();
  market::SetAmount(keywords::kSubsistenceTag, micro::kOneInU,
                    level_.mutable_tags());
  auto* package = level_.add_packages();
  market::SetAmount(kEntertainment, micro::kOneInU,
                    package->mutable_consumed());
  pop_.StartTurn({&level_}, &market_);
  EXPECT_TRUE(pop_.TryProductionStep(dinner_from_fish_, fish_info, &field,
                                     &progress, &market_))
      << fish_info.DebugString();
  EXPECT_EQ(micro::kOneInU, market::GetAmount(pop_.wealth(), kEntertainment));
}

TEST_F(PopulationTest, StartNewProduction) {
  SetupProduction();
  SetupMarket();
  SellGoods(micro::kTenInU, micro::kTenInU, micro::kTenInU);

  industry::decisions::LocalProfitMaximiser evaluator;
  industry::decisions::ProductionContext context;
  industry::decisions::DecisionMap production_info_map;
  geography::proto::Field field;
  context.production_map[dinner_from_fish_proto_->name()] = &dinner_from_fish_;
  context.production_map[dinner_from_grain_proto_->name()] =
      &dinner_from_grain_;
  context.fields.push_back(&field);
  context.market = &market_;
  EXPECT_TRUE(pop_.StartNewProduction(context, &production_info_map, &field));
  auto prod_info = production_info_map.find(&field);
  EXPECT_NE(prod_info, production_info_map.end());
}

} // namespace population
