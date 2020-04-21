#include "games/units/unit.h"

#include <memory>

#include "games/market/goods_utils.h"
#include "games/market/proto/goods.pb.h"
#include "games/units/proto/templates.pb.h"
#include "games/units/proto/units.pb.h"
#include "gtest/gtest.h"
#include "src/google/protobuf/util/message_differencer.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.h"

namespace units {

// UnitTest, lol.
class UnitTest : public testing::Test {
 protected:
  void SetUp() override {
    template_.mutable_template_id()->set_kind("template");
    template_.set_base_action_points_u(micro::kOneInU);
    template_.mutable_mobility()->set_speed_u(1);
    template_.mutable_mobility()->set_max_bulk_u(micro::kOneInU);
    template_.mutable_mobility()->set_max_weight_u(micro::kOneInU);
    Unit::RegisterTemplate(template_);

    unit_proto_.mutable_unit_id()->set_kind("template");
    unit_proto_.mutable_unit_id()->set_number(1);
    unit_ = Unit::FromProto(unit_proto_);
  }

  proto::Template template_;
  proto::Unit unit_proto_;
  std::unique_ptr<Unit> unit_;
};

TEST_F(UnitTest, GetById) {
  util::proto::ObjectId id;
  id.set_kind("template");
  id.set_number(1);
  EXPECT_EQ(unit_.get(), Unit::ById(id));
}

TEST_F(UnitTest, GetTemplate) {
  google::protobuf::util::MessageDifferencer differ;
  const auto* lookup = Unit::TemplateByKind("template");
  EXPECT_NE(nullptr, lookup);
  if (lookup != nullptr) {
    EXPECT_TRUE(differ.Equals(*lookup, template_))
        << lookup->DebugString() << "\ndiffers from\n"
        << template_.DebugString();
  }

  lookup = Unit::TemplateById(template_.template_id());
  EXPECT_NE(nullptr, lookup);
  if (lookup != nullptr) {
    EXPECT_TRUE(differ.Equals(*lookup, template_))
        << lookup->DebugString() << "\ndiffers from\n"
        << template_.DebugString();
  }
}

TEST_F(UnitTest, Capacity) {
  market::proto::TradeGood bulky;
  bulky.set_name("bulky");
  bulky.set_bulk_u(2 * micro::kOneInU);
  bulky.set_weight_u(micro::kHalfInU);
  bulky.set_transport_type(market::proto::TradeGood::TTT_STANDARD);
  market::CreateTradeGood(bulky);
  market::proto::TradeGood heavy;
  heavy.set_name("heavy");
  heavy.set_bulk_u(micro::kHalfInU);
  heavy.set_weight_u(2 * micro::kOneInU);
  heavy.set_transport_type(market::proto::TradeGood::TTT_STANDARD);
  market::CreateTradeGood(heavy);

  EXPECT_EQ(micro::kHalfInU, unit_->Capacity(bulky.name()));
  EXPECT_EQ(micro::kHalfInU, unit_->Capacity(heavy.name()));

  market::SetAmount(bulky.name(), 2 * micro::kOneTenthInU, unit_->mutable_resources());
  EXPECT_EQ(3 * micro::kOneTenthInU, unit_->Capacity(bulky.name()));
  EXPECT_EQ(micro::kOneFourthInU + 2 * micro::kOneTenthInU, unit_->Capacity(heavy.name()));

  market::SetAmount(bulky.name(), micro::kHalfInU, unit_->mutable_resources());
  EXPECT_EQ(0, unit_->Capacity(bulky.name()));
  EXPECT_EQ(0, unit_->Capacity(heavy.name()));
}

TEST_F(UnitTest, ActionPoints) {
  EXPECT_EQ(template_.base_action_points_u(), unit_->action_points_u());
  unit_->use_action_points(micro::kHalfInU);
  EXPECT_EQ(template_.base_action_points_u() - micro::kHalfInU,
            unit_->action_points_u());
  unit_->reset_action_points();
  EXPECT_EQ(template_.base_action_points_u(), unit_->action_points_u());
  unit_->use_action_points(10 * template_.base_action_points_u());
  EXPECT_EQ(0, unit_->action_points_u());
}

} // namespace units

