#include "industry/decisions/production_evaluator.h"

#include "gtest/gtest.h"
#include "geography/proto/geography.pb.h"
#include "industry/industry.h"
#include "industry/proto/decisions.pb.h"
#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"
#include "market/market.h"

namespace industry {
namespace decisions {

class ProductionEvaluatorTest : public testing::Test {
protected:
  void SetUp() override {
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

TEST_F(ProductionEvaluatorTest, LocalProfitMaximiserEvaluate) {
  SetupProduction();
  SetupMarket();
  SellGoods(10, 10, 10);

  LocalProfitMaximiser evaluator;
  ProductionContext context;
  geography::proto::Field field;
  market::proto::Container wealth;
  context.production_map[dinner_from_fish_proto_->name()] = &dinner_from_fish_;
  context.production_map[dinner_from_grain_proto_->name()] = &dinner_from_grain_;
  context.fields.push_back(&field);
  context.market = &market_;
  auto fish_info = evaluator.Evaluate(context, wealth, &field);
  EXPECT_TRUE(fish_info.has_selected()) << fish_info.DebugString();
  EXPECT_FALSE(fish_info.selected().name().empty()) << fish_info.DebugString();
}

}  // namespace decisions
}  // namespace industry
