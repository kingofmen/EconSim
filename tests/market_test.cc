#include "market.h"

#include "proto/goods.pb.h"
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
  EXPECT_DOUBLE_EQ(-1, market.getPrice(kTestGood1));
  market.registerGood(kTestGood1);
  EXPECT_DOUBLE_EQ(1, market.getPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(-1, market.getPrice(kTestGood2));

  Quantity bid;
  bid.set_kind(kTestGood1);
  bid.set_amount(1);
  market.registerBid(bid);

  Quantity offer;
  offer.set_kind(kTestGood1);
  offer.set_amount(1);
  market.registerOffer(offer);

  market.findPrices();
  EXPECT_DOUBLE_EQ(1, market.getPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(1, market.getVolume(kTestGood1));
  
  market.registerBid(bid);
  market.findPrices();
  EXPECT_DOUBLE_EQ(1.25, market.getPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(1, market.getVolume(kTestGood1));
  EXPECT_DOUBLE_EQ(-1, market.getPrice(kTestGood2));

  market.registerOffer(offer);
  market.registerOffer(offer);
  market.findPrices();
  EXPECT_DOUBLE_EQ(1.25 * 0.75, market.getPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(2, market.getVolume(kTestGood1));
}

} // namespace market
