// Tests for worker methods.
#include "industry/worker.h"

#include <memory>

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

  // Production chain that converts labour plus capital to grain.
  Production CapitalToGrain() {
    proto::Production prod_proto;
    prod_proto.set_name(kCapitalToGrain);
    grain_ += micro::kOneInU;
    *prod_proto.mutable_outputs() << grain_;
    auto* step = prod_proto.add_steps();
    auto* input = step->add_variants();
    labour_ += micro::kOneTenthInU;
    market::SetAmount(labour_, input->mutable_consumables());
    capital_ += micro::kHalfInU;
    market::SetAmount(capital_, input->mutable_fixed_capital());
    return Production(prod_proto);
  }

  market::Market market_;
  market::proto::Quantity grain_;
  market::proto::Quantity labour_;
  market::proto::Quantity capital_;
  geography::proto::Field field_;
};

// Check that production scale is correctly calculated.
TEST_F(WorkerTest, CalculateProductionScale) {
  const Production labour = LabourToGrain();
  const Production capital = CapitalToGrain();
  decisions::FieldMap<decisions::proto::ProductionDecision> decisions = {
      {&field_, {}}};
  std::unordered_map<std::string, const Production*> prod_map = {
      {kLabourToGrain, &labour}, {kCapitalToGrain, &capital}};

  decisions::ProductionContext context = {
    &prod_map, {{&field_, NULL}}, {}, &decisions, &market_};
  context.candidates[&field_].emplace_back(
      std::make_unique<decisions::proto::ProductionInfo>());
  context.candidates[&field_].emplace_back(
      std::make_unique<decisions::proto::ProductionInfo>());

  auto* labour_info = context.candidates[&field_][0].get();
  auto* capital_info = context.candidates[&field_][1].get();

  market::proto::Container wealth;
  labour_info->set_name(kLabourToGrain);
  capital_info->set_name(kCapitalToGrain);
  CalculateProductionCosts(labour, market_, field_, labour_info);
  EXPECT_EQ(labour_info->step_info_size(), labour.num_steps());
  CalculateProductionCosts(capital, market_, field_, capital_info);
  EXPECT_EQ(capital_info->step_info_size(), capital.num_steps());

  CalculateProductionScale(wealth, &context, &field_);
  const decisions::proto::VariantInfo& labour_var =
      labour_info->step_info(0).variant(0);
  const decisions::proto::VariantInfo& capital_var =
      capital_info->step_info(0).variant(0);
  EXPECT_EQ(0, labour_var.possible_scale_u());
  EXPECT_EQ(0, capital_var.possible_scale_u());

  labour_ += micro::kOneInU;
  market::SetAmount(labour_, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kOneInU, labour_var.possible_scale_u());
  EXPECT_EQ(0, capital_var.possible_scale_u());

  capital_ += micro::kOneInU;
  market::SetAmount(capital_, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kOneInU, labour_var.possible_scale_u());
  EXPECT_EQ(micro::kOneInU, capital_var.possible_scale_u());
}

// Test scale calculation when consumables, install costs, and capital overlap.
TEST_F(WorkerTest, OverlappingInputs) {
  Production labour = LabourToGrain();
  proto::Input* input = labour.Proto()->mutable_steps(0)->mutable_variants(0);
  market::proto::Container wealth;

  decisions::FieldMap<decisions::proto::ProductionDecision> decisions = {
      {&field_, {}}};
  std::unordered_map<std::string, const Production*> prod_map = {
      {kLabourToGrain, &labour}};
  decisions::ProductionContext context = {
    &prod_map, {{&field_, NULL}}, {}, &decisions, &market_};
  context.candidates[&field_].emplace_back(
      std::make_unique<decisions::proto::ProductionInfo>());
  auto* labour_info = context.candidates[&field_][0].get();

  labour_info->set_name(kLabourToGrain);
  CalculateProductionCosts(labour, market_, field_, labour_info);

  market::SetAmount(labour_.kind(), micro::kOneInU,
                    input->mutable_fixed_capital());
  market::SetAmount(capital_.kind(), micro::kOneInU,
                    input->mutable_fixed_capital());
  market::SetAmount(grain_.kind(), 0, input->mutable_fixed_capital());

  market::SetAmount(labour_.kind(), micro::kOneInU, &wealth);
  market::SetAmount(capital_.kind(), micro::kOneInU, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kHalfInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  EXPECT_EQ(2 * micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).cap_cost_u());

  market::SetAmount(labour_.kind(), micro::kHalfInU,
                    field_.mutable_fixed_capital());
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  EXPECT_EQ(micro::kOneInU + micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).cap_cost_u());

  market::SetAmount(labour_.kind(), micro::kOneInU,
                    input->mutable_install_cost());
  market::SetAmount(labour_.kind(), micro::kOneInU + micro::kOneFourthInU,
                    &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  // Three-fourths units of capital is 1.5, quarter unit of labour is 0.25,
  // three-fourths units labour for the install cost is 0.75, total 2.5.
  EXPECT_EQ(2 * micro::kOneInU + micro::kHalfInU,
            labour_info->step_info(0).variant(0).cap_cost_u());
}

// Test scale calculation for entirely distinct inputs.
TEST_F(WorkerTest, DistinctInputs) {
  proto::Production prod_proto;
  prod_proto.set_name(kLabourToGrain);
  auto* step = prod_proto.add_steps();
  auto* input = step->add_variants();

  grain_ += micro::kOneInU;
  market::SetAmount(grain_, input->mutable_consumables());
  capital_ += micro::kOneInU;
  market::SetAmount(capital_, input->mutable_fixed_capital());
  labour_ += micro::kOneInU;
  market::SetAmount(labour_, input->mutable_install_cost());

  Production labour(prod_proto);
  decisions::FieldMap<decisions::proto::ProductionDecision> decisions = {
      {&field_, {}}};
  std::unordered_map<std::string, const Production*> prod_map = {
      {kLabourToGrain, &labour}};
  decisions::ProductionContext context = {
      &prod_map, {{&field_, NULL}}, {}, &decisions, &market_};
  context.candidates[&field_].emplace_back(
      std::make_unique<decisions::proto::ProductionInfo>());
  auto* labour_info = context.candidates[&field_][0].get();
  labour_info->set_name(kLabourToGrain);
  CalculateProductionCosts(labour, market_, field_, labour_info);

  market::proto::Container wealth;
  market::SetAmount(labour_.kind(), micro::kHalfInU, &wealth);
  market::SetAmount(grain_.kind(), micro::kHundredInU, &wealth);
  market::SetAmount(capital_.kind(), micro::kHundredInU, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kHalfInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  // Capital costs 2, not 1, unit.
  EXPECT_EQ(3 * micro::kHalfInU,
            labour_info->step_info(0).variant(0).cap_cost_u());

  market::SetAmount(labour_.kind(), micro::kHundredInU, &wealth);
  market::SetAmount(grain_.kind(), micro::kOneFourthInU, &wealth);
  market::SetAmount(capital_.kind(), micro::kHundredInU, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kOneFourthInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).cap_cost_u());

  market::SetAmount(labour_.kind(), micro::kHundredInU, &wealth);
  market::SetAmount(grain_.kind(), micro::kHundredInU, &wealth);
  market::SetAmount(capital_.kind(), micro::kThreeFourthsInU, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  EXPECT_EQ(3 * micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).cap_cost_u());

  market::SetAmount(capital_.kind(), micro::kHalfInU,
                    field_.mutable_fixed_capital());
  market::SetAmount(labour_.kind(), micro::kOneFourthInU, &wealth);
  market::SetAmount(grain_.kind(), micro::kHundredInU, &wealth);
  market::SetAmount(capital_.kind(), micro::kHundredInU, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).cap_cost_u());

  market::SetAmount(labour_.kind(), micro::kHundredInU, &wealth);
  market::SetAmount(grain_.kind(), micro::kHundredInU, &wealth);
  market::SetAmount(capital_.kind(), micro::kOneFourthInU, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).cap_cost_u());

  market::SetAmount(labour_.kind(), micro::kHundredInU, &wealth);
  market::SetAmount(grain_.kind(), micro::kThreeFourthsInU, &wealth);
  market::SetAmount(capital_.kind(), micro::kHundredInU, &wealth);
  CalculateProductionScale(wealth, &context, &field_);
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).possible_scale_u());
  EXPECT_EQ(micro::kThreeFourthsInU,
            labour_info->step_info(0).variant(0).cap_cost_u());
}

// Sanity-check unit-cost calculation.
TEST_F(WorkerTest, CalculateProductionCosts) {
  const Production labour = LabourToGrain();
  decisions::proto::ProductionInfo prod_info;
  CalculateProductionCosts(labour, market_, field_, &prod_info);
  EXPECT_EQ(prod_info.step_info_size(), labour.num_steps());
  for (int i = 0; i < prod_info.step_info_size(); ++i) {
    const decisions::proto::StepInfo& step_info = prod_info.step_info(i);
    const proto::ProductionStep& step = labour.get_step(i);
    EXPECT_EQ(step_info.variant_size(), step.variants_size());
    for (int var = 0; var < step_info.variant_size(); ++var) {
      EXPECT_EQ(market_.GetPriceU(step.variants(var).consumables()),
                step_info.variant(var).unit_cost_u());
    }
  }

  const Production capital = CapitalToGrain();
  prod_info.Clear();
  CalculateProductionCosts(capital, market_, field_, &prod_info);
  EXPECT_EQ(prod_info.step_info_size(), capital.num_steps());
  for (int i = 0; i < prod_info.step_info_size(); ++i) {
    const decisions::proto::StepInfo& step_info = prod_info.step_info(i);
    const proto::ProductionStep& step = capital.get_step(i);
    EXPECT_EQ(step_info.variant_size(), step.variants_size());
    for (int var = 0; var < step_info.variant_size(); ++var) {
      EXPECT_EQ(market_.GetPriceU(step.variants(var).consumables()),
                step_info.variant(var).unit_cost_u());
    }
  }
}

// Test that SelectProduction picks a reasonable option.
TEST_F(WorkerTest, SelectProduction) {
  // Test class that selects a Production chain based on its name.
  class TestSelector : public decisions::ProductionEvaluator {
  public:
    void SelectCandidate(decisions::ProductionContext* context,
                         geography::proto::Field* field) const override {
      if (context->candidates.find(field) == context->candidates.end()) {
        return;
      }
      auto& candidates = context->candidates.at(field);
      auto& decision = context->decisions->at(field);
      for (const auto& cand : candidates) {
        if (cand->name() != chain_name) {
          auto* reject = decision.add_rejected();
          *reject = *cand;
          reject->set_reject_reason("Wrong name");
          continue;
        }
        *decision.mutable_selected() = *cand;
      }
    }

    void set_name(const std::string name) { chain_name = name; }

  private:
    std::string chain_name;
  };

  TestSelector evaluator;
  decisions::FieldMap<decisions::proto::ProductionDecision> decisions;
  decisions::ProductionContext context;
  context.decisions = &decisions;

  // Check that all-empty inputs does nothing.
  SelectProduction(evaluator, &context, &field_);
  EXPECT_TRUE(decisions.empty());

  context = {NULL, {{&field_, NULL}}, {}, &decisions, &market_};
  decisions[&field_] = {};
  context.candidates[&field_].emplace_back(
      std::make_unique<decisions::proto::ProductionInfo>());
  context.candidates[&field_].back()->set_name(kLabourToGrain);
  context.candidates[&field_].emplace_back(
      std::make_unique<decisions::proto::ProductionInfo>());
  context.candidates[&field_].back()->set_name(kCapitalToGrain);
  evaluator.set_name(kLabourToGrain);
  SelectProduction(evaluator, &context, &field_);
  decisions::proto::ProductionDecision& decision = decisions[&field_];
  EXPECT_EQ(kLabourToGrain, decision.selected().name());

  decisions[&field_].clear_rejected();
  decisions[&field_].clear_selected();
  evaluator.set_name(kCapitalToGrain);
  SelectProduction(evaluator, &context, &field_);
  decision = decisions[&field_];
  EXPECT_EQ(kCapitalToGrain, decision.selected().name());
}

// Test that TryProductionStep consumes inputs and produces outputs.
TEST_F(WorkerTest, TryLabourToGrain) {
  market::proto::Container source;
  market::proto::Container target;
  market::proto::Container used_capital;

  auto prod = LabourToGrain();
  decisions::proto::StepInfo step_info;
  step_info.set_best_variant(0);
  auto* variant_info = step_info.add_variant();
  variant_info->set_possible_scale_u(micro::kOneInU);
  *field_.mutable_progress() = prod.MakeProgress(micro::kOneInU);

  // First try without labour, expect no result.
  EXPECT_FALSE(TryProductionStep(prod, step_info, &field_,
                                 field_.mutable_progress(), &source, &target,
                                 &used_capital, &market_));
  EXPECT_EQ(0, market::GetAmount(source, grain_));
  EXPECT_EQ(0, market::GetAmount(source, labour_));
  EXPECT_EQ(0, market::GetAmount(target, labour_));
  EXPECT_EQ(0, market::GetAmount(target, grain_));

  // Now with labour.
  labour_ += micro::kOneInU;
  source << labour_;
  EXPECT_TRUE(TryProductionStep(prod, step_info, &field_,
                                field_.mutable_progress(), &source, &target,
                                &used_capital, &market_));
  EXPECT_EQ(0, market::GetAmount(source, grain_));
  EXPECT_EQ(0, market::GetAmount(source, labour_));
  EXPECT_EQ(0, market::GetAmount(target, labour_));
  EXPECT_EQ(micro::kOneInU, market::GetAmount(target, grain_));
}

// Test that InstallFixedCapital actually installs fixed capital.
TEST_F(WorkerTest, InstallFixedCapital) {
  Production capital = CapitalToGrain();
  const proto::Input& input = capital.get_step(0).variants(0);
  market::proto::Container source;
  EXPECT_FALSE(InstallFixedCapital(input, micro::kOneInU, &source,
                                   field_.mutable_fixed_capital(), &market_));
  EXPECT_EQ(0, market::GetAmount(field_.fixed_capital(), "capital"));

  capital_ += micro::kOneInU;
  source << capital_;
  EXPECT_TRUE(InstallFixedCapital(input, micro::kOneInU, &source,
                                  field_.mutable_fixed_capital(), &market_));
  EXPECT_EQ(market::GetAmount(input.fixed_capital(), "capital"),
            market::GetAmount(field_.fixed_capital(), "capital"));

  market::Clear(&source);
  EXPECT_TRUE(InstallFixedCapital(input, micro::kOneInU, &source,
                                  field_.mutable_fixed_capital(), &market_));
  EXPECT_EQ(market::GetAmount(input.fixed_capital(), "capital"),
            market::GetAmount(field_.fixed_capital(), "capital"));

  market::Clear(field_.mutable_fixed_capital());
  capital_ += micro::kOneInU;
  market::SetAmount(capital_, market_.Proto()->mutable_warehouse());
  EXPECT_TRUE(InstallFixedCapital(input, micro::kOneInU, &source,
                                  field_.mutable_fixed_capital(), &market_));
  EXPECT_EQ(market::GetAmount(input.fixed_capital(), "capital"),
            market::GetAmount(field_.fixed_capital(), "capital"));

  market::Clear(field_.mutable_fixed_capital());
  proto::Input with_install_cost = input;
  capital_ += micro::kOneInU;
  labour_ += micro::kOneInU;
  market::SetAmount(capital_, &source);
  market::SetAmount(labour_, &source);
  market::SetAmount(labour_, with_install_cost.mutable_install_cost());

  EXPECT_TRUE(InstallFixedCapital(with_install_cost, micro::kOneInU, &source,
                                  field_.mutable_fixed_capital(), &market_));
  EXPECT_EQ(market::GetAmount(with_install_cost.fixed_capital(), "capital"),
            market::GetAmount(field_.fixed_capital(), "capital"));
  EXPECT_EQ(0, market::GetAmount(source, "labour"));
  EXPECT_EQ(0, market::GetAmount(field_.fixed_capital(), "labour"));
}

} // namespace industry
