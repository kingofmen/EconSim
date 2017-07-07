// Tests for population units.
#include "population/popunit.h"

#include "gtest/gtest.h"
#include "geography/proto/geography.pb.h"
#include "market/market.h"
#include "industry/industry.h"
#include "industry/proto/industry.pb.h"
#include "population/proto/population.pb.h"


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
    culture += 1;
    *pop_.Proto()->mutable_tags() << culture;

    fish_.set_kind("fish");
    house_.set_kind("house");
    youtube_.set_kind("youtube");
    dinner_.set_kind("dinner");
    grain_.set_kind("grain");
    fish_ += 1;
    house_ += 1;
    youtube_ += 1;
    prices_ << fish_;
    prices_ << house_;
    prices_ << youtube_;

    fish_package_ = level_.add_packages();
    house_package_ = level_.add_packages();
    fish_ += 1;
    *fish_package_->mutable_food()->mutable_consumed() << fish_;
    house_ += 1;
    *house_package_->mutable_food()->mutable_consumed() << house_;

    dinner_from_grain_proto_ = dinner_from_grain_.Proto();
    dinner_from_fish_proto_ = dinner_from_fish_.Proto();
  }

  void SetupProduction() {
    dinner_from_fish_proto_->set_name("dinner_from_fish");
    dinner_from_fish_proto_->add_scaling_effects(1);
    dinner_from_fish_proto_->add_scaling_effects(0.8);
    dinner_from_fish_proto_->add_scaling_effects(0.5);
    auto* step = dinner_from_fish_proto_->add_steps();
    auto* variant1 = step->add_variants();
    auto* variant2 = step->add_variants();
    auto* variant3 = step->add_variants();

    house_ += 1;
    fish_ += 1;
    *variant1->mutable_fixed_capital() << house_;
    *variant1->mutable_consumables() << fish_;
    fish_ += 2;
    *variant2->mutable_consumables() << fish_;
    youtube_ += 1;
    *variant3->mutable_consumables() << youtube_;
    dinner_ += 1;
    *dinner_from_fish_proto_->mutable_outputs() << dinner_;

    dinner_from_grain_proto_->set_name("dinner_from_grain");
    dinner_from_grain_proto_->add_scaling_effects(1);
    dinner_from_grain_proto_->add_scaling_effects(0.8);
    step = dinner_from_grain_proto_->add_steps();
    variant1 = step->add_variants();
    variant2 = step->add_variants();

    grain_ += 2;
    *variant1->mutable_consumables() << grain_;

    grain_ += 1;
    house_ += 1;
    *variant2->mutable_consumables() << grain_;
    *variant2->mutable_fixed_capital() << house_;
    dinner_ += 1;
    *dinner_from_grain_proto_->mutable_outputs() << dinner_;
  }

  void SetupMarket() {
    market_.RegisterGood(fish_.kind());
    market_.RegisterGood(youtube_.kind());
    market_.RegisterGood(dinner_.kind());
    market_.RegisterGood(grain_.kind());
    market_.RegisterGood(house_.kind());
    market_.Proto()->set_credit_limit(100);
    market_.Proto()->set_name("market");
    market::SetAmount(fish_.kind(), 1, market_.Proto()->mutable_prices());
    market::SetAmount(youtube_.kind(), 2, market_.Proto()->mutable_prices());
    market::SetAmount(dinner_.kind(), 3, market_.Proto()->mutable_prices());
  }

  void SellGoods(double fish = 0, double youtube = 0, double grain = 0) {
    fish_.set_amount(fish);
    youtube_.set_amount(youtube);
    grain_.set_amount(grain);
    seller_ += fish_;
    seller_ += youtube_;
    seller_ += grain_;
    EXPECT_DOUBLE_EQ(market_.TryToSell(fish_, &seller_), fish);
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
  // Pop has no resources, can afford nothing.
  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), nullptr);

  fish_ += 1;
  *pop_.Proto()->mutable_wealth() << fish_;
  // Pop can now eat fish.
  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), fish_package_);

  house_ += 1;
  *pop_.Proto()->mutable_wealth() << house_;
  fish_ += 1;
  prices_ << fish_;
  // Fish is now more expensive than house.
  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), house_package_);

  market::proto::Quantity scots;
  scots.set_kind(kTestCulture2);
  scots += 0.1;
  *house_package_->mutable_required_tags() << scots;
  // House is now forbidden.
  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), fish_package_);

  market::proto::Quantity culture;
  culture.set_kind(kTestCulture1);
  culture += 0.1;
  *fish_package_->mutable_required_tags() << culture;
  market::Clear(house_package_->mutable_required_tags());
  culture += 0.1;
  *house_package_->mutable_required_tags() << culture;
  // Both packages are allowed again, so it'll be house.
  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), house_package_);
}

TEST_F(PopulationTest, Consume) {
  fish_ += 1;
  *pop_.Proto()->mutable_wealth() << fish_;
  // Pop can now eat fish.
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);

  house_ += 1;
  *pop_.Proto()->mutable_wealth() << house_;
  fish_ += 1;
  prices_ << fish_;
  fish_ += 1;
  *pop_.Proto()->mutable_wealth() << fish_;
  // Fish is now more expensive than house.
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), house_), 0);

  market::proto::Quantity tools;
  tools.set_kind("tools");
  tools += 1;
  *house_package_->mutable_shelter()->mutable_capital() << tools;
  house_ += 1;
  *pop_.Proto()->mutable_wealth() << house_;

  // No tools, eat the fish.
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), house_), 1);

  // With tools available, use them.
  tools += 1;
  fish_ += 1;
  *pop_.Proto()->mutable_wealth() << fish_ << tools;
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), house_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), tools), 1);
}

TEST_F(PopulationTest, ConsumptionTags) {
  auto& fish_tags = *fish_package_->mutable_tags();
  market::proto::Quantity bad_breath;
  bad_breath.set_kind("halitosis");
  bad_breath += 1;
  fish_tags << bad_breath;

  market::proto::Quantity satiation;
  satiation.set_kind("wellfedness");
  satiation += 1;
  *level_.mutable_tags() << satiation;

  fish_ += 1;
  *pop_.Proto()->mutable_wealth() << fish_;
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->tags(), bad_breath), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->tags(), satiation), 1);
}

TEST_F(PopulationTest, AutoProduction) {
  proto::AutoProduction labour;
  market::proto::Quantity work;
  work.set_kind("labour");
  work += 1;
  *labour.mutable_output() << work;

  proto::AutoProduction prayer;
  market::proto::Quantity words;
  words.set_kind("kneeling");
  words += 1;
  *prayer.mutable_output() << words;

  work += 2;
  words += 1;
  market::proto::Container prices;
  prices << work;
  prices << words;

  // Prayer is cheaper, so the pop should choose to work.
  std::vector<const proto::AutoProduction*> production = {&labour, &prayer};
  pop_.AutoProduce(production, prices);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), work), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), words), 0);

  market::proto::Quantity culture;
  culture.set_kind(kTestCulture2);
  culture += 1;
  *labour.mutable_required_tags() << culture;

  // Pop can no longer work, so now it should pray.
  pop_.AutoProduce(production, prices);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), work), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), words), 1);  

  culture += 1.1;
  *pop_.Proto()->mutable_tags() << culture;
  // With ability to work restored, pop should work again.
  pop_.AutoProduce(production, prices);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), work), 2);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), words), 1);  
}

TEST_F(PopulationTest, EndTurn) {
  fish_ += 1;
  *pop_.Proto()->mutable_wealth() << fish_;
  youtube_ += 1;
  *pop_.Proto()->mutable_wealth() << youtube_;

  prices_.Clear();
  // Fish and guests stink after three days.
  prices_ << fish_;
  // A good cat video is a thing of joy forever.
  youtube_ += 1;
  prices_ << youtube_;
  
  pop_.EndTurn(prices_);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), fish_), 0);
  EXPECT_FALSE(market::Contains(pop_.Proto()->wealth(), fish_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.Proto()->wealth(), youtube_), 1);
}

TEST_F(PopulationTest, GetStepInfo) {
  SetupProduction();
  SetupMarket();
  market::proto::Container stores;
  fish_.set_amount(0.5);
  youtube_.set_amount(5);
  stores += fish_;
  stores += youtube_;
  market_.TryToSell(fish_, &stores);
  market_.TryToSell(youtube_, &stores);

  geography::proto::Field field;
  PopUnit::ProductionStepInfo step_info;
  auto progress = dinner_from_fish_.MakeProgress(3);

  pop_.GetStepInfo(dinner_from_fish_, market_, field, progress, &step_info);

  EXPECT_EQ(step_info.variants.size(), 3);
  EXPECT_DOUBLE_EQ(step_info.variants[0].unit_cost, 0);
  EXPECT_DOUBLE_EQ(step_info.variants[0].possible_scale, 0);

  EXPECT_DOUBLE_EQ(step_info.variants[1].unit_cost, 2);
  EXPECT_DOUBLE_EQ(step_info.variants[1].possible_scale, 0.25);

  EXPECT_DOUBLE_EQ(step_info.variants[2].unit_cost, 2);
  EXPECT_DOUBLE_EQ(step_info.variants[2].possible_scale, 3);
}

TEST_F(PopulationTest, TryProductionStep) {
  SetupProduction();
  SetupMarket();
  geography::proto::Field field;
  PopUnit::ProductionStepInfo step_info;
  auto progress = dinner_from_fish_.MakeProgress(3);
  EXPECT_DOUBLE_EQ(2.8, dinner_from_fish_.Efficiency(progress));

  EXPECT_FALSE(pop_.TryProductionStep(dinner_from_fish_, &field, &progress,
                                      &market_, &step_info));
  EXPECT_EQ(step_info.attempts_this_turn, 1);

  SellGoods(2, 5);

  // Process makes no profit because it has no output.
  dinner_from_fish_proto_->mutable_outputs()->Clear();
  EXPECT_FALSE(pop_.TryProductionStep(dinner_from_fish_, &field, &progress,
                                      &market_, &step_info));
  EXPECT_EQ(step_info.attempts_this_turn, 2);

  auto entertainment = market::MakeQuantity("entertainment", 10);
  market_.RegisterGood(entertainment.kind());
  market::SetAmount(entertainment, market_.Proto()->mutable_prices());
  entertainment.set_amount(1);
  market::SetAmount(entertainment, dinner_from_fish_proto_->mutable_outputs());
  EXPECT_DOUBLE_EQ(dinner_from_fish_.Efficiency(progress),
                   market::GetAmount(dinner_from_fish_.ExpectedOutput(progress),
                                     entertainment));

  EXPECT_TRUE(pop_.TryProductionStep(dinner_from_fish_, &field, &progress,
                                     &market_, &step_info));
  EXPECT_EQ(step_info.attempts_this_turn, 3);
  EXPECT_DOUBLE_EQ(1, market::GetAmount(pop_.Proto()->wealth(), entertainment));
  EXPECT_TRUE(dinner_from_fish_.Complete(progress));

  EXPECT_FALSE(pop_.TryProductionStep(dinner_from_fish_, &field, &progress,
                                      &market_, &step_info));
}

TEST_F(PopulationTest, StartNewProduction) {
  SetupProduction();
  SetupMarket();
  SellGoods(10, 10, 10);

  PopUnit::ProductionMap chains;
  chains[dinner_from_fish_proto_->name()] = &dinner_from_fish_;
  chains[dinner_from_grain_proto_->name()] = &dinner_from_grain_;
  geography::proto::Field field;
  
  EXPECT_TRUE(pop_.StartNewProduction(chains, market_, &field));
}

} // namespace population
