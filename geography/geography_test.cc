#include "geography/geography.h"

#include <memory>

#include "geography/proto/geography.pb.h"
#include "gtest/gtest.h"
#include "industry/proto/industry.pb.h"
#include "industry/industry.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "util/status/macros.h"

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
     production_ = std::unique_ptr<industry::Production>(
         new industry::Production());
     stuff_.set_kind("stuff");
     area_proto_ = area_.Proto();
     field_ = area_proto_->add_fields();
   }

   std::unique_ptr<industry::Production> production_;
   Area area_;
   proto::Area* area_proto_;
   proto::Field* field_;
   market::proto::Quantity stuff_;
};

TEST_F(GeographyTest, TestFilters) {
  EXPECT_TRUE(HasLandType(*field_, *production_));
  production_->Proto()->set_land_type(industry::proto::LT_ORCHARDS);
  EXPECT_FALSE(HasLandType(*field_, *production_));
  field_->set_land_type(industry::proto::LT_ORCHARDS);
  EXPECT_TRUE(HasLandType(*field_, *production_));

  auto* step = production_->Proto()->add_steps();
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
  auto& resource_limits = *area_proto_->mutable_limits();
  auto& limits = *resource_limits.mutable_maximum();
  stuff_ += 10;
  limits << stuff_;

  auto& recovery = *resource_limits.mutable_recovery();
  stuff_ += 10;
  recovery << stuff_;

  *field_->mutable_progress() = production_->MakeProgress(1.0);

  auto& resources = *field_->mutable_resources();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 0);
  area_.Update();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 10);
  area_.Update();
  EXPECT_DOUBLE_EQ(market::GetAmount(resources, stuff_), 10);
}

TEST_F(GeographyTest, FallowRecovery) {
  auto& resource_limits = *area_proto_->mutable_limits();
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

TEST_F(GeographyTest, Transition) {
  proto::Transition transition;
  transition.set_source(industry::proto::LT_ORCHARDS);
  transition.set_target(industry::proto::LT_BUILT);
  transition.set_steps(1);
  stuff_ += 10;
  *transition.mutable_final_fixed_capital() << stuff_;

  market::proto::Quantity fences;
  fences.set_kind("fences");
  fences += 1;
  *transition.mutable_step_fixed_capital() << fences;

  market::proto::Quantity labour;
  labour.set_kind("labour");
  labour += 1;
  *transition.mutable_step_input() << labour;

  EXPECT_FALSE(
      GenerateTransitionProcess(*field_, transition, production_.get()).ok());
  field_->set_land_type(industry::proto::LT_ORCHARDS);
  EXPECT_OK(GenerateTransitionProcess(*field_, transition, production_.get()));
  EXPECT_EQ(1, production_->Proto()->steps_size());
  EXPECT_DOUBLE_EQ(
      market::GetAmount(production_->Proto()->steps(0).variants(0).consumables(), labour),
      1.0);

  fences += 1;
  *field_->mutable_fixed_capital() << fences;
  production_->Proto()->Clear();
  EXPECT_OK(GenerateTransitionProcess(*field_, transition, production_.get()));
  EXPECT_DOUBLE_EQ(
      market::GetAmount(
          production_->Proto()->steps(0).variants(0).consumables(), labour),
      1.0);
  EXPECT_DOUBLE_EQ(
      market::GetAmount(
          production_->Proto()->steps(1).variants(0).consumables(), labour),
      1.0);
  EXPECT_EQ(2, production_->Proto()->steps_size());
}

}  // namespace geography
