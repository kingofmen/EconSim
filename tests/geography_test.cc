#include "geography/geography.h"

#include <memory>

#include "geography/proto/geography.pb.h"
#include "industry/proto/industry.pb.h"
#include "industry/industry.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "gtest/gtest.h"

namespace geography {
namespace {
constexpr char kTestGood1[] = "TestGood1";
constexpr char kTestGood2[] = "TestGood2";
}

using market::proto::Container;
using market::proto::Quantity;

class GeographyTest : public testing::Test {
 protected:
  void SetUp() override {
    production_ = std::unique_ptr<industry::proto::Production>(new industry::proto::Production());
    progress_ = std::unique_ptr<industry::Progress>(
        new industry::Progress(production_.get()));
    stuff_.set_kind("stuff");
    field_ = area_.add_fields();
  }

  std::unique_ptr<industry::Progress> progress_;
  std::unique_ptr<industry::proto::Production> production_;
  Area area_;
  proto::Field* field_;
  market::proto::Quantity stuff_;
};

TEST_F(GeographyTest, TestFilters) {
  EXPECT_TRUE(HasLandType(*field_, *production_));
  production_->set_land_type(industry::proto::LT_ORCHARDS);
  EXPECT_FALSE(HasLandType(*field_, *production_));
  field_->set_land_type(industry::proto::LT_ORCHARDS);
  EXPECT_TRUE(HasLandType(*field_, *production_));

  auto* step = production_->add_steps();
  auto* variant = step->add_variants();
  EXPECT_TRUE(HasFixedCapital(*field_, *production_));
  auto& capital = *variant->mutable_fixed_capital();
  
  stuff_ += 1;
  capital << stuff_;
  EXPECT_FALSE(HasFixedCapital(*field_, *production_));
  auto& field_capital = *field_->mutable_fixed_capital();
  stuff_ += 1;
  field_capital << stuff_;
  EXPECT_TRUE(HasFixedCapital(*field_, *production_));

  EXPECT_TRUE(HasRawMaterials(*field_, *production_));
  auto& raw_materials = *variant->mutable_raw_materials();
  stuff_ += 1;
  raw_materials << stuff_;
  EXPECT_FALSE(HasRawMaterials(*field_, *production_));
  auto& field_materials = *field_->mutable_resources();
  stuff_ += 1;
  field_materials << stuff_;
  EXPECT_TRUE(HasRawMaterials(*field_, *production_));
}

TEST_F(GeographyTest, Recovery) {
  auto& resource_limits = *area_.mutable_limits();
  auto& limits = *resource_limits.mutable_maximum();
  stuff_ += 10;
  limits << stuff_;

  auto& recovery = *resource_limits.mutable_recovery();
  stuff_ += 10;
  recovery << stuff_;

  *field_->mutable_production() = *progress_;

  auto& resources = *field_->mutable_resources();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 0);
  area_.Update();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 10);
  area_.Update();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 10);
}

TEST_F(GeographyTest, FallowRecovery) {
  auto& resource_limits = *area_.mutable_limits();
  auto& limits = *resource_limits.mutable_maximum();
  stuff_ += 10;
  limits << stuff_;

  auto& fallow_recovery = *resource_limits.mutable_fallow_recovery();
  stuff_ += 10;
  fallow_recovery << stuff_;

  auto& resources = *field_->mutable_resources();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 0);
  area_.Update();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 10);
  area_.Update();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 10);
}

}  // namespace geography
