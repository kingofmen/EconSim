#include "industry/industry.h"

#include <memory>

#include "industry/proto/industry.pb.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "gtest/gtest.h"

namespace industry {
namespace {
constexpr char kTestGood1[] = "TestGood1";
constexpr char kTestGood2[] = "TestGood2";
constexpr char kTestGood3[] = "TestGood3";
}

using market::proto::Container;
using market::proto::Quantity;

class IndustryTest : public testing::Test {
 protected:
  void SetUp() override {
    wool_.set_kind(kTestGood1);
    cloth_.set_kind(kTestGood2);
    mules_.set_kind(kTestGood3);
    production_ = std::unique_ptr<Production>(new Production());
    prod_proto_ = production_->Proto();
    prod_proto_->set_name("spinning_wool");
  }

  proto::ProductionStep* AddWoolStep() {
    auto* step = prod_proto_->add_steps();
    auto* input = step->add_variants();
    auto& consumables = *input->mutable_consumables();
    wool_ += 1;
    consumables << wool_;
    return step;
  }
  void AddCapitalVariant(proto::ProductionStep* step) {
    auto* variant = step->add_variants();
    auto& consumables = *variant->mutable_consumables();
    wool_ += 0.5;
    consumables << wool_;
    auto& fixed_capital = *variant->mutable_fixed_capital();
    mules_ += 1;
    fixed_capital << mules_;
  }

  void AddClothOutput() {
    auto& output = *prod_proto_->mutable_outputs();
    cloth_ += 1;
    output << cloth_;
  }

  proto::Progress progress_;
  std::unique_ptr<Production> production_;
  proto::Production* prod_proto_;
  Container inputs_;
  Container outputs_;
  Container capital_;
  Container raw_materials_;
  Container existing_;
  Quantity wool_;
  Quantity cloth_;
  Quantity mules_;
};

TEST_F(IndustryTest, EmptyIsComplete) {
  progress_ = production_->MakeProgress(1.0);
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
}

TEST_F(IndustryTest, OneStep) {
  AddWoolStep();
  progress_ = production_->MakeProgress(1.0);
  EXPECT_FALSE(production_->Complete(progress_));
  AddClothOutput();

  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_FALSE(production_->Complete(progress_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));
  EXPECT_FALSE(market::Contains(outputs_, cloth_));

  wool_ += 1;
  inputs_ << wool_;
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_TRUE(market::Contains(outputs_, cloth_));
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_),
                   market::GetAmount(prod_proto_->outputs(), cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));
}

TEST_F(IndustryTest, TwoSteps) {
  AddWoolStep();
  AddWoolStep();
  AddClothOutput();
  progress_ = production_->MakeProgress(1.0);

  wool_ += 1;
  inputs_ << wool_;
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_FALSE(production_->Complete(progress_));
  EXPECT_FALSE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));

  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_FALSE(production_->Complete(progress_));
  EXPECT_FALSE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));

  wool_ += 1;
  inputs_ << wool_;
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_TRUE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_),
                   market::GetAmount(prod_proto_->outputs(), cloth_));
}

TEST_F(IndustryTest, MovableCapital) {
  AddWoolStep();
  AddClothOutput();
  progress_ = production_->MakeProgress(1.0);

  auto* step = prod_proto_->mutable_steps(0);
  auto* input = step->mutable_variants(0);
  auto* capital = input->mutable_movable_capital();

  Quantity dogs;
  dogs.set_kind("dogs");
  dogs += 1;
  *capital << dogs;
  dogs += 1;
  wool_ += 1;
  inputs_ << wool_ << dogs;

  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, dogs), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, MovableCapitalSameAsInput) {
  AddWoolStep();
  AddClothOutput();
  progress_ = production_->MakeProgress(1.0);

  auto* step = prod_proto_->mutable_steps(0);
  auto* input = step->mutable_variants(0);
  auto* capital = input->mutable_movable_capital();
  wool_ += 1;
  *capital << wool_;
  wool_ += 2;
  inputs_ << wool_;

  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, FixedCapital) {
  AddWoolStep();
  AddClothOutput();
  progress_ = production_->MakeProgress(1.0);

  auto* step = prod_proto_->mutable_steps(0);
  auto* input = step->mutable_variants(0);
  auto* capital = input->mutable_fixed_capital();

  Quantity dogs;
  dogs.set_kind("dogs");
  dogs += 1;
  *capital << dogs;
  wool_ += 1;
  inputs_ << wool_;

  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_FALSE(production_->Complete(progress_));

  dogs += 1;
  capital_ << dogs;
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);

  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(capital_, dogs), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, InstitutionalCapital) {
  AddWoolStep();
  AddClothOutput();
  progress_ = production_->MakeProgress(1.0);

  prod_proto_->set_experience_effect(0.5);

  wool_ += 0.5;
  inputs_ << wool_;
  production_->PerformStep(capital_, 1.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, ScalingEffects) {
  AddWoolStep();
  AddClothOutput();
  prod_proto_->add_scaling_effects(0.9);
  progress_ = production_->MakeProgress(2.0);
  wool_ += 1;
  inputs_ << wool_;

  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_FALSE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 1);

  wool_ += 1;
  inputs_ << wool_;
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1.9);

  prod_proto_->add_scaling_effects(0.8);
  progress_.set_scaling(2.5);
  wool_ += 2;
  inputs_ << wool_;
  outputs_ >> cloth_;
  progress_.set_step(0);
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_FALSE(production_->Complete(progress_));

  wool_ += 0.5;
  inputs_ << wool_;
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_),
                   1.0 + 0.9 + sqrt(0.5) * 0.8);
}

TEST_F(IndustryTest, SkippingEffects) {
  AddWoolStep();
  AddWoolStep();
  AddClothOutput();
  progress_ = production_->MakeProgress(1.0);
  prod_proto_->mutable_steps(0)->set_skip_effect(0.5);

  wool_ += 1;
  inputs_ << wool_;

  production_->Skip(&progress_);
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 0.5);
}

TEST_F(IndustryTest, RawMaterials) {
  AddWoolStep();
  AddClothOutput();
  progress_ = production_->MakeProgress(1.0);
  auto &raw_material = *prod_proto_->mutable_steps(0)
                            ->mutable_variants(0)
                            ->mutable_raw_materials();
  Quantity clay;
  clay.set_kind("clay");
  clay += 1;
  raw_material << clay;
  wool_ += 1;
  inputs_ << wool_;

  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 1.0);
  
  clay += 1;
  raw_materials_ << clay;
  production_->PerformStep(capital_, 0.0, 0, &inputs_, &raw_materials_, &outputs_, &progress_);
  EXPECT_TRUE(production_->Complete(progress_));
  EXPECT_DOUBLE_EQ(market::GetAmount(raw_materials_, clay), 0.0);
}

} // namespace industry
