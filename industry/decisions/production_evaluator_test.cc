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
  }

  void SetPrices(market::Measure fish, market::Measure salt, market::Measure gold) {
    fish_.set_amount(fish);
    salt_.set_amount(salt);
    gold_.set_amount(gold);
    market::SetAmount(fish_, market_.Proto()->mutable_prices_u());
    market::SetAmount(salt_, market_.Proto()->mutable_prices_u());
    market::SetAmount(gold_, market_.Proto()->mutable_prices_u());
  }

  market::Market market_;
  market::proto::Quantity fish_;
  market::proto::Quantity salt_;
  market::proto::Quantity gold_;
};

TEST_F(ProductionEvaluatorTest, LocalProfitMaximiser) {
  SetPrices(micro::kOneInU, micro::kOneInU * 5, micro::kOneInU * 10);
  ProductionContext context;
  context.market = &market_;
  std::vector<proto::ProductionInfo> candidates(3);

  proto::StepInfo* step_info;
  proto::VariantInfo* var_info;

  candidates[0].set_name("fish");
  candidates[0].set_max_scale_u(micro::kOneInU * 2);
  fish_.set_amount(micro::kOneInU * 3);
  *candidates[0].mutable_expected_output() << fish_;
  step_info = candidates[0].add_step_info();
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info->set_possible_scale_u(micro::kOneInU * 2);

  step_info = candidates[0].add_step_info();
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info->set_possible_scale_u(micro::kOneInU * 2);

  candidates[1].set_name("salt");
  candidates[1].set_max_scale_u(micro::kOneInU);
  salt_.set_amount(micro::kOneInU);
  *candidates[1].mutable_expected_output() << salt_;
  step_info = candidates[1].add_step_info();
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info->set_possible_scale_u(micro::kOneInU);

  candidates[2].set_name("gold");
  candidates[2].set_max_scale_u(micro::kOneInU);
  gold_.set_amount(micro::kOneInU);
  *candidates[2].mutable_expected_output() << gold_;
  step_info = candidates[2].add_step_info();
  var_info = step_info->add_variant();
  var_info->set_unit_cost_u(micro::kOneInU);
  var_info->set_possible_scale_u(micro::kOneInU);

  LocalProfitMaximiser evaluator;
  proto::ProductionDecision decision;
  evaluator.SelectCandidate(context, candidates, &decision);
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

} // namespace decisions
} // namespace industry
