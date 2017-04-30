#include "market/market.h"

#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "market/proto/market.pb.h"
#include "gtest/gtest.h"

namespace market {

namespace {
constexpr char kTestGood1[] = "TestGood1";
constexpr char kTestGood2[] = "TestGood2";
constexpr char kSilver[] = "silver";
constexpr char kMarketName[] = "market";
constexpr char kCredit[] = "market_short_term_credit";
constexpr char kDebt[] = "market_short_term_debt";
}

using proto::Quantity;
using proto::Container;

TEST(MarketTest, FindPrices) {
  Market market;
  market.set_legal_tender(kSilver);
  market.set_name(kMarketName);
  EXPECT_DOUBLE_EQ(-1, market.GetPrice(kTestGood1));

  market.RegisterGood(kTestGood1);
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(-1, market.GetPrice(kTestGood2));

  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));

  Quantity bid;
  Container bidder;
  bid.set_kind(kTestGood1);
  bid.set_amount(1);
  market.RegisterBid(bid, &bidder);

  Quantity offer;
  Container seller;
  offer.set_kind(kTestGood1);
  offer.set_amount(1);
  market.RegisterOffer(offer, &seller);

  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(0, market.GetVolume(kTestGood1));

  market.RegisterBid(bid, &bidder);
  market.RegisterOffer(offer, &seller);
  Add(kTestGood1, 1, &seller);
  Add(kSilver, 1, &bidder);
  market.TryToBuy(bid, &bidder);
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(1, GetAmount(seller, kSilver));
  EXPECT_DOUBLE_EQ(1, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));

  market.RegisterBid(bid, &bidder);
  market.RegisterBid(bid, &bidder);
  market.RegisterOffer(offer, &seller);
  Add(kTestGood1, 1, &seller);
  Add(kSilver, 1, &bidder);
  market.TryToBuy(bid, &bidder);
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1.25, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(2, GetAmount(seller, kSilver));
  EXPECT_DOUBLE_EQ(2, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));

  SetAmount(kTestGood1, 1, market.mutable_prices());
  market.RegisterBid(bid, &bidder);
  market.RegisterOffer(offer, &seller);
  market.RegisterOffer(offer, &seller);
  Add(kTestGood1, 1, &seller);
  Add(kSilver, 1, &bidder);
  market.TryToBuy(bid, &bidder);
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(0.75, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(3, GetAmount(seller, kSilver));
  EXPECT_DOUBLE_EQ(3, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));

  SetAmount(kTestGood1, 1, market.mutable_prices());
  market.RegisterBid(bid, &bidder);
  market.RegisterOffer(offer, &seller);
  Add(kTestGood1, 1, &seller);
  Add(kSilver, 1, &bidder);
  market.TryToSell(bid, &seller);
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(4, GetAmount(seller, kSilver));
  EXPECT_DOUBLE_EQ(4, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));
}

TEST(MarketTest, TransferMoney) {
  Market market;
  market.set_name(kMarketName);
  market.set_legal_tender(kSilver);
  market.set_credit_limit(100);
  Container rich;
  Container poor;

  SetAmount(kSilver, 1, &rich);
  market.TransferMoney(1, &rich, &poor);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kSilver), 0);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kSilver), 1);

  SetAmount(kCredit, 1, &rich);
  market.TransferMoney(1, &rich, &poor);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kCredit), 1);
  
  SetAmount(kCredit, 1, &rich);
  SetAmount(kCredit, 0, &poor);
  SetAmount(kDebt, 0.5, &poor);
  market.TransferMoney(1, &rich, &poor);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kCredit), 0.5);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kDebt), 0);

  SetAmount(kDebt, 1, &poor);
  market.TransferMoney(1, &rich, &poor);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kDebt), 1);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kCredit), 0.5);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kDebt), 0);

  market.TransferMoney(1, &rich, &poor);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kDebt), 2);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kCredit), 1.5);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kDebt), 0);

  Clear(&rich);
  Clear(&poor);
  SetAmount(kCredit, 1, &rich);
  SetAmount(kSilver, 1, &rich);
  SetAmount(kDebt, 1, &poor);
  market.TransferMoney(4, &rich, &poor);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kSilver), 0);
  EXPECT_DOUBLE_EQ(GetAmount(rich, kDebt), 2);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kCredit), 2);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kSilver), 1);
  EXPECT_DOUBLE_EQ(GetAmount(poor, kDebt), 0);
}

} // namespace market
