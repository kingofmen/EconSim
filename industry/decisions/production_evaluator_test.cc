#include "industry/decisions/production_evaluator.h"

#include "industry/proto/decisions.pb.h"
#include "market/goods_utils.h"
#include "market/market.h"
#include "util/arithmetic/microunits.h"
#include "gtest/gtest.h"

namespace industry {
namespace decisions {

class ProductionEvaluatorTest : public testing::Test {
protected:
  void SetUp() override {
    fish_.set_kind("fish");
    salt_.set_kind("salt");
    gold_.set_kind("gold");
    market_.RegisterGood(fish_.kind());
    market_.RegisterGood(salt_.kind());
    market_.RegisterGood(gold_.kind());

    context_.market = &market_;
    context_.decisions = &decisionMap_;
    decisionMap_[&field_] = proto::ProductionDecision();
    context_.candidates.insert(
        {&field_, std::vector<std::unique_ptr<proto::ProductionInfo>>()});
  }

  void SetPrices(market::Measure fish, market::Measure salt, market::Measure gold) {
    fish_.set_amount(fish);
    salt_.set_amount(salt);
    gold_.set_amount(gold);
    market::SetAmount(fish_, market_.Proto()->mutable_prices_u());
    market::SetAmount(salt_, market_.Proto()->mutable_prices_u());
    market::SetAmount(gold_, market_.Proto()->mutable_prices_u());
  }

  ProductionContext context_;
  DecisionMap decisionMap;
  geography::proto::Field field_;
  market::Market market_;
  market::proto::Quantity fish_;
  market::proto::Quantity salt_;
  market::proto::Quantity gold_;
  DecisionMap decisionMap_;
};

TEST_F(ProductionEvaluatorTest, LocalProfitMaximiser) {
  SetPrices(micro::kOneInU, micro::kOneInU * 5, micro::kOneInU * 10);
  auto& decision = context_.decisions->at(&field_);
  auto& candidates = context_.candidates[&field_];
  candidates.emplace_back(std::make_unique<proto::ProductionInfo>());
  candidates.emplace_back(std::make_unique<proto::ProductionInfo>());
  candidates.emplace_back(std::make_unique<proto::ProductionInfo>());
  proto::StepInfo* step_info;
  proto::VariantInfo* var_info;

  proto::ProductionInfo* candidate = candidates[0].get();
  candidate->set_name("fish");
  candidate->set_max_scale_u(micro::kOneInU * 2);
  fish_.set_amount(micro::kOneInU * 3);
  *candidate->mutable_expected_output() << fish_;
  step_info = candidate->add_step_info();
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info->set_possible_scale_u(micro::kOneInU * 2);

  step_info = candidate->add_step_info();
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info->set_possible_scale_u(micro::kOneInU * 2);

  candidate = candidates[1].get();
  candidate->set_name("salt");
  candidate->set_max_scale_u(micro::kOneInU);
  salt_.set_amount(micro::kOneInU);
  *candidate->mutable_expected_output() << salt_;
  step_info = candidate->add_step_info();
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info->set_possible_scale_u(micro::kOneInU);

  candidate = candidates[2].get();
  candidate->set_name("gold");
  candidate->set_max_scale_u(micro::kOneInU);
  gold_.set_amount(micro::kOneInU);
  *candidate->mutable_expected_output() << gold_;
  step_info = candidate->add_step_info();
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info->set_possible_scale_u(micro::kOneInU);

  LocalProfitMaximiser evaluator;
  evaluator.SelectCandidate(&context_, &field_);
  EXPECT_EQ("gold", decision.selected().name()) << decision.DebugString();
  EXPECT_EQ(2, decision.rejected_size()) << decision.DebugString();
  const proto::ProductionInfo& rejected1 = decision.rejected(0);
  EXPECT_EQ("fish", rejected1.name()) << decision.DebugString();
  EXPECT_EQ("Unprofitable, revenue 3000000 vs cost 4000000",
            rejected1.reject_reason())
      << decision.DebugString();

  const proto::ProductionInfo& rejected2 = decision.rejected(1);
  EXPECT_EQ("salt", rejected2.name()) << decision.DebugString();
  EXPECT_EQ("Less profit than gold", rejected2.reject_reason())
      << decision.DebugString();
}

TEST_F(ProductionEvaluatorTest, FieldSpecifier) {
  SetPrices(micro::kOneInU, micro::kOneInU * 5, micro::kOneInU * 10);
  auto& decision = context_.decisions->at(&field_);
  auto& candidates = context_.candidates[&field_];
  candidates.emplace_back(std::make_unique<proto::ProductionInfo>());
  candidates.emplace_back(std::make_unique<proto::ProductionInfo>());

  proto::ProductionInfo* candidate;
  candidate = candidates[0].get();
  candidate->set_name("fish");
  candidate = candidates[1].get();
  candidate->set_name("salt");

  class Fallback : public ProductionEvaluator {
   public:
    void SelectCandidate(ProductionContext* context,
                         geography::proto::Field* field) const override {
      auto& decision = context->decisions->at(field);
      decision.mutable_selected()->set_name("fallback");
    }
  };

  Fallback fallback;
  FieldSpecifier evaluator(&fallback);
  evaluator.SelectCandidate(&context_, &field_);
  EXPECT_EQ(decision.selected().name(), "fallback");
  evaluator.SetFieldProduction(&field_, "salt");
  evaluator.SelectCandidate(&context_, &field_);
  EXPECT_EQ(decision.selected().name(), "salt");
  evaluator.SetFieldProduction(&field_, "fish");
  evaluator.SelectCandidate(&context_, &field_);
  EXPECT_EQ(decision.selected().name(), "fish");
  evaluator.SetFieldProduction(&field_, "nonesuch");
  evaluator.SelectCandidate(&context_, &field_);
  EXPECT_EQ(decision.selected().name(), "fallback");
  evaluator.SetFieldProduction(&field_, "");
  evaluator.SelectCandidate(&context_, &field_);
  EXPECT_EQ(decision.selected().name(), "fallback");
}

} // namespace decisions
} // namespace industry
