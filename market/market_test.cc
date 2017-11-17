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
  market.Proto()->set_legal_tender(kSilver);
  market.Proto()->set_name(kMarketName);
  market.Proto()->set_credit_limit(100);
  EXPECT_DOUBLE_EQ(-1, market.GetPrice(kTestGood1));

  market.RegisterGood(kTestGood1);
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(-1, market.GetPrice(kTestGood2));

  // No bids or offer, price should be unchanged.
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));

  Quantity bid = MakeQuantity(kTestGood1, 1);
  Container bidder;
  EXPECT_DOUBLE_EQ(0, market.TryToBuy(bid, &bidder));

  Quantity offer = MakeQuantity(kTestGood1, 1);
  Container seller;
  EXPECT_DOUBLE_EQ(0, market.TryToSell(offer, &seller));

  // Bids exactly match offers, but are not effectual. No change.
  EXPECT_DOUBLE_EQ(0, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));

  seller += offer;
  // Seller's goods go into the warehouse in exchange for credit.
  EXPECT_DOUBLE_EQ(1, market.TryToSell(offer, &seller));
  EXPECT_DOUBLE_EQ(1, GetAmount(seller, kCredit));
  EXPECT_DOUBLE_EQ(1, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_DOUBLE_EQ(1, GetAmount(market.Proto()->warehouse(), kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));

  // Buyer can buy the goods, going into debt.
  EXPECT_DOUBLE_EQ(1, market.TryToBuy(bid, &bidder));
  EXPECT_DOUBLE_EQ(1, GetAmount(bidder, kDebt));
  EXPECT_DOUBLE_EQ(1, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(market.Proto()->warehouse(), kTestGood1));

  // Bids match offers and went through. No change.
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));

  // Try to buy twice.
  Add(kTestGood1, 1, &seller);
  Add(kSilver, 1, &bidder);
  EXPECT_DOUBLE_EQ(1, market.TryToSell(offer, &seller));
  EXPECT_DOUBLE_EQ(1, market.TryToBuy(bid, &bidder));
  EXPECT_DOUBLE_EQ(0, market.TryToBuy(bid, &bidder));
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(1, GetAmount(market.Proto()->warehouse(), kSilver));
  EXPECT_DOUBLE_EQ(2, GetAmount(seller, kCredit));
  EXPECT_DOUBLE_EQ(2, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));
  EXPECT_DOUBLE_EQ(1, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(market.Proto()->warehouse(), kTestGood1));
  // More bids than offers, but bids are not effectual. No change.
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1.25, market.GetPrice(kTestGood1));

  // Sell twice.
  SetAmount(kTestGood1, 1, market.Proto()->mutable_prices());
  SetAmount(kTestGood1, 1, &seller);
  EXPECT_EQ(0, market.TryToBuy(bid, &bidder));
  EXPECT_EQ(1, market.TryToSell(offer, &seller));
  EXPECT_EQ(0, market.TryToSell(offer, &seller));
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  // More offers than bids, but not effectual. No change.
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(3, GetAmount(seller, kCredit));
  EXPECT_DOUBLE_EQ(3, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));
  EXPECT_DOUBLE_EQ(1, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(market.Proto()->warehouse(), kTestGood1));
  EXPECT_DOUBLE_EQ(2, GetAmount(bidder, kDebt));

  // Buy twice, now with money.
  SetAmount(kTestGood1, 1, market.Proto()->mutable_prices());
  SetAmount(kTestGood1, 1, &seller);
  SetAmount(kSilver, 2, &bidder);
  EXPECT_EQ(0, market.TryToBuy(bid, &bidder));
  EXPECT_EQ(1, market.TryToSell(offer, &seller));
  EXPECT_EQ(0, market.TryToBuy(bid, &bidder));
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  // More bids than offers, and they are effectual. Price should rise.
  market.FindPrices();
  EXPECT_DOUBLE_EQ(1.25, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(3, GetAmount(seller, kCredit));
  EXPECT_DOUBLE_EQ(1, GetAmount(seller, kSilver));
  EXPECT_DOUBLE_EQ(1, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(4, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));
  EXPECT_DOUBLE_EQ(1, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(market.Proto()->warehouse(), kTestGood1));
  EXPECT_DOUBLE_EQ(2, GetAmount(bidder, kDebt));

  // Sell twice, now with goods.
  SetAmount(kTestGood1, 1, market.Proto()->mutable_prices());
  SetAmount(kTestGood1, 2, &seller);
  market.TryToSell(offer, &seller);
  market.TryToBuy(bid, &bidder);
  market.TryToSell(offer, &seller);

  // More offers than bids, and they are effectual. Price should fall.
  EXPECT_DOUBLE_EQ(1, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_DOUBLE_EQ(0.75, market.GetPrice(kTestGood1));
  EXPECT_DOUBLE_EQ(3, GetAmount(seller, kCredit));
  EXPECT_DOUBLE_EQ(3, GetAmount(seller, kSilver));
  EXPECT_DOUBLE_EQ(0, GetAmount(bidder, kSilver));
  EXPECT_DOUBLE_EQ(5, GetAmount(bidder, kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(seller, kTestGood1));
  EXPECT_DOUBLE_EQ(1, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_DOUBLE_EQ(0, GetAmount(market.Proto()->warehouse(), kSilver));
  EXPECT_DOUBLE_EQ(1, GetAmount(market.Proto()->warehouse(), kTestGood1));
  EXPECT_DOUBLE_EQ(2, GetAmount(bidder, kDebt));
}

TEST(MarketTest, TransferMoney) {
  Market market;
  market.Proto()->set_name(kMarketName);
  market.Proto()->set_legal_tender(kSilver);
  market.Proto()->set_credit_limit(100);
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
  market.Proto()->set_legal_tender(kSilver);
  market.Proto()->set_name(kMarketName);
  market.Proto()->set_credit_limit(100);
  market.RegisterGood(kTestGood1);
  market.RegisterGood(kTestGood2);
  Container seller;
  SetAmount(kTestGood1, 0.5, &seller);
  SetAmount(kTestGood1, 1, market.Proto()->mutable_prices());

  Quantity offer = MakeQuantity(kTestGood1, 0.5);
  EXPECT_DOUBLE_EQ(0.5, market.TryToSell(offer, &seller));
  EXPECT_DOUBLE_EQ(0.5, market.AvailableImmediately(kTestGood1));
  EXPECT_DOUBLE_EQ(0.5, GetAmount(seller, kCredit));
  EXPECT_DOUBLE_EQ(0.5, GetAmount(market.Proto()->market_debt(), kTestGood1));

  EXPECT_DOUBLE_EQ(0, market.TryToSell(offer, &seller));
  EXPECT_DOUBLE_EQ(0.5, market.AvailableImmediately(kTestGood1));
  EXPECT_DOUBLE_EQ(0.5, GetAmount(seller, kCredit));
  EXPECT_DOUBLE_EQ(0.5, GetAmount(market.Proto()->market_debt(), kTestGood1));
}

TEST(MarketTest, BuySellBuy) {
  Market market;
  market.Proto()->set_legal_tender(kSilver);
  market.Proto()->set_name(kMarketName);
  market.Proto()->set_credit_limit(0);
  market.RegisterGood(kTestGood1);
  SetAmount(kTestGood1, 1, market.Proto()->mutable_prices());

  Container buyer;
  Container seller;
  SetAmount(kTestGood1, 2.0, &seller);

  Quantity offer = MakeQuantity(kTestGood1, 1.0);
  // Offer to buy, no goods available.
  EXPECT_DOUBLE_EQ(0.0, market.TryToBuy(offer, &buyer));
  // Offer to sell, buyer has no money.
  SetAmount(kSilver, 1, market.Proto()->mutable_warehouse());
  EXPECT_DOUBLE_EQ(1.0, market.TryToSell(offer, &seller));
  // Give buyer some money.
  SetAmount(kSilver, 1.0, &buyer);
  EXPECT_DOUBLE_EQ(1.0, market.TryToBuy(offer, &buyer));

  // Create some additional supply.
  SetAmount(kSilver, 1, market.Proto()->mutable_warehouse());
  EXPECT_DOUBLE_EQ(1.0, market.TryToSell(offer, &seller));
  // Buy offer should have disappeared, so price should drop.
  market.FindPrices();
  EXPECT_DOUBLE_EQ(0.75, market.GetPrice(kTestGood1));
}

} // namespace market
