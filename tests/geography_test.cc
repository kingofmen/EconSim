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
  }

  std::unique_ptr<industry::Progress> progress_;
  std::unique_ptr<industry::proto::Production> production_;
  Field field_;
};

TEST_F(GeographyTest, TestFilters) {
  EXPECT_TRUE(field_.HasLandType(*production_));
  production_->set_land_type(industry::proto::LT_ORCHARDS);
  EXPECT_FALSE(field_.HasLandType(*production_));
  field_.set_land_type(industry::proto::LT_ORCHARDS);
  EXPECT_TRUE(field_.HasLandType(*production_));

  auto* step = production_->add_steps();
  auto* variant = step->add_variants();
  EXPECT_TRUE(field_.HasFixedCapital(*production_));
  auto& capital = *variant->mutable_fixed_capital();
  market::proto::Quantity stuff;
  stuff.set_kind("stuff");
  stuff += 1;
  capital << stuff;
  EXPECT_FALSE(field_.HasFixedCapital(*production_));
  auto& field_capital = *field_.mutable_fixed_capital();
  stuff += 1;
  field_capital << stuff;
  EXPECT_TRUE(field_.HasFixedCapital(*production_));

  EXPECT_TRUE(field_.HasRawMaterials(*production_));
  auto& raw_materials = *variant->mutable_raw_materials();
  stuff += 1;
  raw_materials << stuff;
  EXPECT_FALSE(field_.HasRawMaterials(*production_));
  auto& field_materials = *field_.mutable_resources();
  stuff += 1;
  field_materials << stuff;
  EXPECT_TRUE(field_.HasRawMaterials(*production_));
}

}  // namespace geography
