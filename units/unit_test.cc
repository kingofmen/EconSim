#include "units/unit.h"

#include <memory>

#include "gtest/gtest.h"
#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "units/proto/templates.pb.h"
#include "units/proto/units.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.h"

namespace units {

// UnitTest, lol.
class UnitTest : public testing::Test {
 protected:
  void SetUp() override {
    template_.set_id(1);
    template_.mutable_mobility()->set_speed_u(1);
    template_.mutable_mobility()->set_max_bulk_u(micro::kOneInU);
    template_.mutable_mobility()->set_max_weight_u(micro::kOneInU);
    template_.set_name("test_unit");
    Unit::RegisterTemplate(template_);

    unit_proto_.mutable_unit_id()->set_type(1);
    unit_proto_.mutable_unit_id()->set_number(1);
    unit_ = Unit::FromProto(unit_proto_);
  }

  proto::Template template_;
  proto::Unit unit_proto_;
  std::unique_ptr<Unit> unit_;
};

TEST_F(UnitTest, GetById) {
  util::proto::ObjectId id;
  id.set_type(1);
  id.set_number(1);
  EXPECT_EQ(unit_.get(), Unit::ById(id));
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

} // namespace units

