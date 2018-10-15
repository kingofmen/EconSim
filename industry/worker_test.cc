// Tests for worker methods.
#include "industry/worker.h"

#include "geography/proto/geography.pb.h"
#include "industry/industry.h"
#include "industry/proto/decisions.pb.h"
#include "industry/proto/industry.pb.h"
#include "market/market.h"
#include "util/arithmetic/microunits.h"
#include "gtest/gtest.h"

namespace industry {

namespace {
constexpr char kLabourToGrain[] = "labour_to_grain";
constexpr char kCapitalToGrain[] = "capital_to_grain";
} // namespace


class WorkerTest : public testing::Test {
protected:
  void SetUp() override {
    grain_.set_kind("grain");
    labour_.set_kind("labour");
    capital_.set_kind("capital");
    market_.RegisterGood(grain_.kind());
    market_.RegisterGood(labour_.kind());
    market_.RegisterGood(capital_.kind());
    market_.Proto()->set_credit_limit(micro::kHundredInU);
    market_.Proto()->set_name("test_market");

    auto* prices = market_.Proto()->mutable_prices_u();
    market::SetAmount(grain_.kind(), 2 * micro::kOneInU, prices);
    market::SetAmount(labour_.kind(), 1 * micro::kOneInU, prices);
    market::SetAmount(capital_.kind(), 2 * micro::kOneInU, prices);
  }

  // Simple production chain that converts labour into grain.
  Production LabourToGrain() {
    proto::Production prod_proto;
    prod_proto.set_name(kLabourToGrain);
    grain_ += micro::kOneInU;
    *prod_proto.mutable_outputs() << grain_;
    auto* step = prod_proto.add_steps();
    auto* input = step->add_variants();
    labour_ += micro::kOneInU;
    *input->mutable_consumables() << labour_;
    return Production(prod_proto);
  }

  // {roduction chain that converts labour plus capital to grain.
  Production CapitalToGrain() {
    proto::Production prod_proto;
    prod_proto.set_name(kCapitalToGrain);
    grain_ += micro::kOneInU;
    *prod_proto.mutable_outputs() << grain_;
    auto* step = prod_proto.add_steps();
    auto* input = step->add_variants();
    labour_ += micro::kOneInU / 10;
    *input->mutable_consumables() << labour_;
    capital_ += micro::kOneInU / 2;
    *input->mutable_fixed_capital() << capital_;
    return Production(prod_proto);
  }

  market::Market market_;
  market::proto::Quantity grain_;
  market::proto::Quantity labour_;
  market::proto::Quantity capital_;
  geography::proto::Field field_;
};

// Test that SelectProduction picks a reasonable option.
TEST_F(WorkerTest, SelectProduction) {
  decisions::ProductionContext context;
  std::vector<ProductionFilter*> filters;
  market::proto::Container source;  
  decisions::LocalProfitMaximiser evaluator;
  decisions::DecisionMap info_map;

  // Check that all-empty inputs does nothing.
  SelectProduction(context, source, filters, evaluator, &info_map);
  EXPECT_TRUE(info_map.empty());

  // Check that we can select the only available option.
  context = {{}, {&field_}, &market_};
  auto labour_to_grain = LabourToGrain();
  context.production_map[kLabourToGrain] = &labour_to_grain;
  labour_ += micro::kOneInU;
  source << labour_;
  SelectProduction(context, source, filters, evaluator, &info_map);
  auto decision = info_map[&field_];
  EXPECT_EQ(kLabourToGrain, decision.selected().name());

  // Using capital would be more profitable if we had any.
  auto capital_to_grain = CapitalToGrain();
  context.production_map[kCapitalToGrain] = &capital_to_grain;
  SelectProduction(context, source, filters, evaluator, &info_map);
  decision = info_map[&field_];
  EXPECT_EQ(kLabourToGrain, decision.selected().name());

  // Supply some capital.
  capital_ += micro::kOneInU;
  *field_.mutable_fixed_capital() << capital_;
  SelectProduction(context, source, filters, evaluator, &info_map);
  decision = info_map[&field_];
  EXPECT_EQ(kCapitalToGrain, decision.selected().name());

  // Filter out the capital option and go back to labour.
  class CapitalFilter : public ProductionFilter {
   public:
     bool Filter(const geography::proto::Field&,
                 const Production& prod) const override {
       return prod.get_name() != kCapitalToGrain;
     }
  };
  CapitalFilter capital_filter;
  filters.push_back(&capital_filter);
  SelectProduction(context, source, filters, evaluator, &info_map);
  decision = info_map[&field_];
  EXPECT_EQ(kLabourToGrain, decision.selected().name());
}

// Test that TryProductionStep consumes inputs and produces outputs.
TEST_F(WorkerTest, TryLabourToGrain) {
  market::proto::Container source;
  market::proto::Container target;
  market::proto::Container used_capital;

  auto prod = LabourToGrain();
  decisions::proto::ProductionInfo prod_info;
  auto* step_info = prod_info.add_step_info();
  step_info->set_best_variant(0);
  auto* variant_info = step_info->add_variant();
  variant_info->set_possible_scale_u(micro::kOneInU);
  *field_.mutable_progress() = prod.MakeProgress(micro::kOneInU);

  // First try without labour, expect no result.
  EXPECT_FALSE(TryProductionStep(prod, prod_info, &field_,
                                field_.mutable_progress(), &source, &target,
                                &used_capital, &market_));
  EXPECT_EQ(0, market::GetAmount(source, grain_));
  EXPECT_EQ(0, market::GetAmount(source, labour_));
  EXPECT_EQ(0, market::GetAmount(target, labour_));
  EXPECT_EQ(0, market::GetAmount(target, grain_));

  // Now with labour.
  labour_ += micro::kOneInU;
  source << labour_;
  EXPECT_TRUE(TryProductionStep(prod, prod_info, &field_,
                                field_.mutable_progress(), &source, &target,
                                &used_capital, &market_));
  EXPECT_EQ(0, market::GetAmount(source, grain_));
  EXPECT_EQ(0, market::GetAmount(source, labour_));
  EXPECT_EQ(0, market::GetAmount(target, labour_));
  EXPECT_EQ(micro::kOneInU, market::GetAmount(target, grain_));
}

}  // namespace industry
