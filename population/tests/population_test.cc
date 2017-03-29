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
    pop_.set_culture(kTestCulture1);
    fish_.set_kind("fish");
    house_.set_kind("house");
    youtube_.set_kind("youtube");

    fish_ += 1;
    house_ += 1;
    youtube_ += 1;
    prices_ << fish_;
    prices_ << house_;
    prices_ << youtube_;
  }

  PopUnit pop_;
  market::proto::Quantity fish_;
  market::proto::Quantity house_;
  market::proto::Quantity youtube_;
  market::proto::Container prices_;
};

TEST_F(PopulationTest, CheapestPackage) {
  proto::ConsumptionLevel level_;
  auto* package1 = level_.add_packages();
  package1->mutable_allowed_cultures()->insert({kTestCulture1, true});
  auto* package2 = level_.add_packages();
  package2->mutable_allowed_cultures()->insert({kTestCulture1, true});
  fish_ += 1;
  *package1->mutable_food()->mutable_consumed() << fish_;
  house_ += 1;
  *package2->mutable_food()->mutable_consumed() << house_;

  EXPECT_EQ(pop_.CheapestPackage(level_, prices_), nullptr);

}

} // namespace population
