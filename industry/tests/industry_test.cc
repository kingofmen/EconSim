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
}

using market::proto::Container;
using market::proto::Quantity;

class IndustryTest : public testing::Test {
 protected:
  void SetUp() override {
    wool_.set_kind(kTestGood1);
    cloth_.set_kind(kTestGood2);
    production_ = std::unique_ptr<proto::Production>(new proto::Production());
    progress_ = std::unique_ptr<Progress>(
        new Progress(production_.get()));
  }

  void AddWoolStep() {
    auto* step = production_->add_steps();
    auto* input = step->add_variants();
    auto& consumables = *input->mutable_consumables();
    wool_ += 1;
    consumables << wool_;
  }

  void AddClothOutput() {
    auto& output = *production_->mutable_outputs();
    cloth_ += 1;
    output << cloth_;
  }

  std::unique_ptr<Progress> progress_;
  std::unique_ptr<proto::Production> production_;
  Container inputs_;
  Container outputs_;
  Container capital_;
  Container raw_materials_;
  Quantity wool_;
  Quantity cloth_;
};

TEST_F(IndustryTest, EmptyIsComplete) {
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
}

TEST_F(IndustryTest, OneStep) {
  AddWoolStep();
  EXPECT_FALSE(progress_->Complete());
  AddClothOutput();

  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_FALSE(progress_->Complete());
  EXPECT_FALSE(market::Contains(outputs_, wool_));
  EXPECT_FALSE(market::Contains(outputs_, cloth_));

  wool_ += 1;
  inputs_ << wool_;
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_TRUE(market::Contains(outputs_, cloth_));
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_),
                   market::GetAmount(production_->outputs(), cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));
}

TEST_F(IndustryTest, TwoSteps) {
  AddWoolStep();
  AddWoolStep();
  AddClothOutput();

  wool_ += 1;
  inputs_ << wool_;
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_FALSE(progress_->Complete());
  EXPECT_FALSE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));

  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_FALSE(progress_->Complete());
  EXPECT_FALSE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));

  wool_ += 1;
  inputs_ << wool_;
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_TRUE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_),
                   market::GetAmount(production_->outputs(), cloth_));
}

TEST_F(IndustryTest, MovableCapital) {
  AddWoolStep();
  AddClothOutput();

  auto* step = production_->mutable_steps(0);
  auto* input = step->mutable_variants(0);
  auto* capital = input->mutable_movable_capital();

  Quantity dogs;
  dogs.set_kind("dogs");
  dogs += 1;
  *capital << dogs;
  dogs += 1;
  wool_ += 1;
  inputs_ << wool_ << dogs;

  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, dogs), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, MovableCapitalSameAsInput) {
  AddWoolStep();
  AddClothOutput();

  auto* step = production_->mutable_steps(0);
  auto* input = step->mutable_variants(0);
  auto* capital = input->mutable_movable_capital();
  wool_ += 1;
  *capital << wool_;
  wool_ += 2;
  inputs_ << wool_;

  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, FixedCapital) {
  AddWoolStep();
  AddClothOutput();

  auto* step = production_->mutable_steps(0);
  auto* input = step->mutable_variants(0);
  auto* capital = input->mutable_fixed_capital();

  Quantity dogs;
  dogs.set_kind("dogs");
  dogs += 1;
  *capital << dogs;
  wool_ += 1;
  inputs_ << wool_;

  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_FALSE(progress_->Complete());

  dogs += 1;
  capital_ << dogs;
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);

  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(capital_, dogs), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, InstitutionalCapital) {
  AddWoolStep();
  AddClothOutput();

  production_->set_experience_effect(0.5);

  wool_ += 0.5;
  inputs_ << wool_;
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_, 1.0);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, ScalingEffects) {
  AddWoolStep();
  AddClothOutput();
  production_->add_scaling_effects(0.9);
  wool_ += 1;
  inputs_ << wool_;

  progress_->set_scaling(2);
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_FALSE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 1);

  wool_ += 1;
  inputs_ << wool_;
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1.9);

  production_->add_scaling_effects(0.8);
  progress_->set_scaling(2.5);
  wool_ += 2;
  inputs_ << wool_;
  outputs_ >> cloth_;
  progress_->set_step(0);
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_FALSE(progress_->Complete());

  wool_ += 0.5;
  inputs_ << wool_;
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_),
                   1.0 + 0.9 + sqrt(0.5) * 0.8);
}

TEST_F(IndustryTest, SkippingEffects) {
  AddWoolStep();
  AddWoolStep();
  AddClothOutput();
  production_->mutable_steps(0)->set_skip_effect(0.5);

  wool_ += 1;
  inputs_ << wool_;

  progress_->Skip();
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 0.5);
}

TEST_F(IndustryTest, RawMaterials) {
  AddWoolStep();
  AddClothOutput();
  auto &raw_material = *production_->mutable_steps(0)
                            ->mutable_variants(0)
                            ->mutable_raw_materials();
  Quantity clay;
  clay.set_kind("clay");
  clay += 1;
  raw_material << clay;
  wool_ += 1;
  inputs_ << wool_;

  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 1.0);
  
  clay += 1;
  raw_materials_ << clay;
  progress_->PerformStep(capital_, &inputs_, &raw_materials_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(raw_materials_, clay), 0.0);
}

} // namespace industry
