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

  std::unique_ptr<Progress> progress_;
  std::unique_ptr<proto::Production> production_;
  Container inputs_;
  Container outputs_;
  Quantity wool_;
  Quantity cloth_;
};

TEST_F(IndustryTest, EmptyIsComplete) {
  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
}

TEST_F(IndustryTest, OneStep) {
  auto* step = production_->add_steps();
  EXPECT_FALSE(progress_->Complete());

  auto* input = step->add_variants();
  auto& consumables = *input->mutable_consumables();
  auto& output = *production_->mutable_outputs();

  wool_ += 1;
  consumables << wool_;

  cloth_ += 1;
  output << cloth_;

  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_FALSE(progress_->Complete());
  EXPECT_FALSE(market::Contains(outputs_, wool_));
  EXPECT_FALSE(market::Contains(outputs_, cloth_));

  wool_ += 1;
  inputs_ << wool_;
  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_TRUE(market::Contains(outputs_, cloth_));
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_),
                   market::GetAmount(output, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));
}

TEST_F(IndustryTest, TwoSteps) {
  auto* step = production_->add_steps();
  auto* input = step->add_variants();
  auto* consumables = input->mutable_consumables();
  wool_ += 1;
  *consumables << wool_;

  step = production_->add_steps();
  input = step->add_variants();
  consumables = input->mutable_consumables();
  wool_ += 1;
  *consumables << wool_;

  auto& output = *production_->mutable_outputs();
  cloth_ += 1;
  output << cloth_;

  wool_ += 1;
  inputs_ << wool_;
  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_FALSE(progress_->Complete());
  EXPECT_FALSE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));

  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_FALSE(progress_->Complete());
  EXPECT_FALSE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));

  wool_ += 1;
  inputs_ << wool_;
  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_TRUE(market::Contains(outputs_, cloth_));
  EXPECT_FALSE(market::Contains(outputs_, wool_));
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_),
                   market::GetAmount(output, cloth_));
}

TEST_F(IndustryTest, MovableCapital) {
  auto* step = production_->add_steps();
  auto* input = step->add_variants();
  auto* consumables = input->mutable_consumables();
  auto* capital = input->mutable_movable_capital();
  wool_ += 1;
  *consumables << wool_;
  Quantity dogs;
  dogs.set_kind("dogs");
  dogs += 1;
  *capital << dogs;
  dogs += 1;
  wool_ += 1;
  inputs_ << wool_ << dogs;

  auto& output = *production_->mutable_outputs();
  cloth_ += 1;
  output << cloth_;

  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, dogs), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, MovableCapitalSameAsInput) {
  auto* step = production_->add_steps();
  auto* input = step->add_variants();
  auto* consumables = input->mutable_consumables();
  auto* capital = input->mutable_movable_capital();
  wool_ += 1;
  *consumables << wool_;
  wool_ += 1;
  *capital << wool_;
  wool_ += 2;
  inputs_ << wool_;

  auto& output = *production_->mutable_outputs();
  cloth_ += 1;
  output << cloth_;

  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 1);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1);
}

TEST_F(IndustryTest, ScalingEffects) {
  auto* step = production_->add_steps();
  auto* input = step->add_variants();
  auto* consumables = input->mutable_consumables();
  production_->add_scaling_effects(1.0);
  production_->add_scaling_effects(1.9);
  wool_ += 1;
  *consumables << wool_;
  wool_ += 1;
  inputs_ << wool_;

  auto& output = *production_->mutable_outputs();
  cloth_ += 1;
  output << cloth_;

  progress_->set_scaling(2);
  progress_->PerformStep(&inputs_, &outputs_);
  EXPECT_TRUE(progress_->Complete());
  EXPECT_DOUBLE_EQ(market::GetAmount(inputs_, wool_), 0);
  EXPECT_DOUBLE_EQ(market::GetAmount(outputs_, cloth_), 1.9);
}

} // namespace industry
