#include "market/market.h"

#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "market/proto/market.pb.h"
#include "gtest/gtest.h"

namespace market {

namespace {
constexpr char kTestGood1[] = "TestGood1";
constexpr char kTestGood2[] = "TestGood2";
}

using proto::Quantity;
using proto::Container;

TEST(MarketTest, FindPrices) {
  Market market;
  EXPECT_DOUBLE_EQ(-1, market.GetPrice(kTestGood1));

  market.RegisterGood(kTestGood1);
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(-1, market.GetPrice(kTestGood2));

  Quantity bid;
  bid.set_kind(kTestGood1);
  bid.set_amount(1);
  market.RegisterBid(bid);

  Quantity offer;
  offer.set_kind(kTestGood1);
  offer.set_amount(1);
  market.RegisterOffer(offer);

  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));

  market.RegisterBid(bid);
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1.25, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  EXPECT_DOUBLE_EQ(-1, market.GetPrice(kTestGood2));

  market.RegisterOffer(offer);
  market.RegisterOffer(offer);
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1.25 * 0.75, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(2, market.GetVolume(kTestGood1));
}

TEST(MarketTest, Protos) {
  proto::MarketProto proto;
  *proto.mutable_goods() << kTestGood1;
  Market market = proto;
  Quantity bid;
  bid.set_kind(kTestGood1);
  bid.set_amount(1);
  market.RegisterBid(bid);

  bid.set_kind(kTestGood2);
  market.RegisterBid(bid);

  proto = market;
  EXPECT_EQ(1, proto.goods().quantities().size());
  EXPECT_DOUBLE_EQ(1, GetAmount(proto.bids(), kTestGood1));
  EXPECT_FALSE(Contains(proto.bids(), kTestGood2));
}

} // namespace market
