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

  // No bids or offer, price should be unchanged.
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

  // Bids exactly match offers, but are not effectual. No change.
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(0, market.GetVolume(kTestGood1));

  market.RegisterBid(bid, &bidder);
  EXPECT_DOUBLE_EQ(0, market.AvailableToBuy(kTestGood1));
  market.RegisterOffer(offer, &seller);
  EXPECT_DOUBLE_EQ(1, market.AvailableToBuy(kTestGood1));
  Add(kTestGood1, 1, &seller);
  Add(kSilver, 1, &bidder);
  market.TryToBuy(bid, &bidder);
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  EXPECT_DOUBLE_EQ(0, market.AvailableToBuy(kTestGood1));
  market.FindPrices();
  // Bids match offers and are effectual. No change.
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
  // More bids than offers, but bids are not effectual. No change.
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
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
  // More offers than bids, but not effectual. No change.
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
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

  SetAmount(kTestGood1, 1, market.mutable_prices());
  market.RegisterBid(bid, &bidder);
  market.RegisterBid(bid, &bidder);
  market.RegisterOffer(offer, &seller);
  Add(kTestGood1, 1, &seller);
  Add(kSilver, 2, &bidder);
  market.TryToSell(bid, &seller);
  // More bids than offers, and they are effectual. Price should rise.
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1.25, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(1, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(5, GetAmount(seller, kSilver));
  EXPECT_DOUBLE_EQ(5, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));

  SetAmount(kTestGood1, 1, market.mutable_prices());
  market.RegisterBid(bid, &bidder);
  market.RegisterOffer(offer, &seller);
  market.RegisterOffer(offer, &seller);
  Add(kTestGood1, 2, &seller);
  market.TryToSell(bid, &seller);
  // More offers than bids, and they are effectual. Price should fall.
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(0.75, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(6, GetAmount(seller, kSilver));
  EXPECT_DOUBLE_EQ(6, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(1, GetAmount(seller, kTestGood1));
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

TEST(MarketTest, Available) {
  Market market;
  market.RegisterGood(kTestGood1);
  market.RegisterGood(kTestGood2);
  Container seller;
  SetAmount(kTestGood1, 1, &seller);
  Quantity offer;
  offer.set_kind(kTestGood1);
  offer.set_amount(1);
  market.RegisterOffer(offer, &seller);

  EXPECT_DOUBLE_EQ(1, market.AvailableToBuy(kTestGood1));
  Container basket;
  SetAmount(kTestGood1, 1, &basket);
  EXPECT_TRUE(market.AvailableToBuy(basket));
  SetAmount(kTestGood1, 2, &basket);
  EXPECT_FALSE(market.AvailableToBuy(basket));

  market.RegisterOffer(offer, &seller);
  EXPECT_TRUE(market.AvailableToBuy(basket));

  SetAmount(kTestGood2, 1, &basket);
  EXPECT_FALSE(market.AvailableToBuy(basket));

  offer.set_kind(kTestGood2);
  market.RegisterOffer(offer, &seller);
  EXPECT_TRUE(market.AvailableToBuy(basket));
}

} // namespace market
