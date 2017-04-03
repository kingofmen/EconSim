// Tests for population units.
#include "population/popunit.h"

#include "gtest/gtest.h"
#include "population/proto/population.pb.h"


namespace population {
namespace {
constexpr char kTestCulture1[] = "Norwegian";
constexpr char kTestCulture2[] = "Scots";
} // namespace

class PopulationTest : public testing::Test {
protected:
  void SetUp() override {
    pop_.add_males(1);
    market::proto::Quantity culture;
    culture.set_kind(kTestCulture1);
    culture += 1;
    *pop_.mutable_tags() << culture;

    fish_.set_kind("fish");
    house_.set_kind("house");
    youtube_.set_kind("youtube");

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
  }

  PopUnit pop_;
  proto::ConsumptionLevel level_;
  proto::ConsumptionPackage* fish_package_;
  proto::ConsumptionPackage* house_package_;
  market::proto::Quantity fish_;
  market::proto::Quantity house_;
  market::proto::Quantity youtube_;
  market::proto::Container prices_;
};

TEST_F(PopulationTest, CheapestPackage) {
  // Pop has no resources, can afford nothing.
  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), nullptr);

  fish_ += 1;
  *pop_.mutable_wealth() << fish_;
  // Pop can now eat fish.
  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), fish_package_);

  house_ += 1;
  *pop_.mutable_wealth() << house_;
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
  market::Clear(*house_package_->mutable_required_tags());
  culture += 0.1;
  *house_package_->mutable_required_tags() << culture;
  // Both packages are allowed again, so it'll be house.
  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), house_package_);
}

TEST_F(PopulationTest, Consume) {
  fish_ += 1;
  *pop_.mutable_wealth() << fish_;
  // Pop can now eat fish.
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), fish_), 0);

  house_ += 1;
  *pop_.mutable_wealth() << house_;
  fish_ += 1;
  prices_ << fish_;
  fish_ += 1;
  *pop_.mutable_wealth() << fish_;
  // Fish is now more expensive than house.
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), fish_), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), house_), 0);

  market::proto::Quantity tools;
  tools.set_kind("tools");
  tools += 1;
  *house_package_->mutable_shelter()->mutable_capital() << tools;
  house_ += 1;
  *pop_.mutable_wealth() << house_;

  // No tools, eat the fish.
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), fish_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), house_), 1);

  // With tools available, use them.
  tools += 1;
  fish_ += 1;
  *pop_.mutable_wealth() << fish_ << tools;
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), fish_), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), house_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), tools), 1);
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
  *pop_.mutable_wealth() << fish_;
  EXPECT_TRUE(pop_.Consume(level_, prices_));
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), fish_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.tags(), bad_breath), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.tags(), satiation), 1);
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
  std::vector<proto::AutoProduction*> production = {&labour, &prayer};
  pop_.AutoProduce(production, prices);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), work), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), words), 0);

  market::proto::Quantity culture;
  culture.set_kind(kTestCulture2);
  culture += 1;
  *labour.mutable_required_tags() << culture;

  // Pop can no longer work, so now it should pray.
  pop_.AutoProduce(production, prices);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), work), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), words), 1);  

  culture += 1.1;
  *pop_.mutable_tags() << culture;
  // With ability to work restored, pop should work again.
  pop_.AutoProduce(production, prices);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), work), 2);
  EXPECT_DOUBLE_EQ(market::GetAmount(pop_.wealth(), words), 1);  
}

} // namespace population
