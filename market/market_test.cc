#include "market/market.h"

#include "market/goods_utils.h"
#include "market/proto/goods.pb.h"
#include "market/proto/market.pb.h"
#include "gtest/gtest.h"
#include "util/arithmetic/microunits.h"

namespace market {

namespace {
using proto::Quantity;
using proto::Container;

constexpr char kTestGood1[] = "TestGood1";
constexpr char kTestGood2[] = "TestGood2";
constexpr char kSilver[] = "silver";
constexpr char kMarketName[] = "market";
constexpr char kCredit[] = "market_short_term_credit";
constexpr char kDebt[] = "market_short_term_debt";

}

class MarketTest : public testing::Test {
 protected:
  void SetUp() override {
    market_.Proto()->set_legal_tender(kSilver);
    market_.Proto()->set_name(kMarketName);
    market_.Proto()->set_credit_limit(micro::kHundredInU);
  }

  void SetPrice(const Quantity& price) {
    SetAmount(price, market_.Proto()->mutable_prices_u());
  }

  void SetPrice(const std::string name, const Measure amount) {
    SetAmount(name, amount, market_.Proto()->mutable_prices_u());
  }

  Market market_;
  Container buyer_;
};

TEST_F(MarketTest, FindPrices) {
  Market market;
  market.Proto()->set_legal_tender(kSilver);
  market.Proto()->set_name(kMarketName);
  market.Proto()->set_credit_limit(micro::kHundredInU);
  EXPECT_EQ(micro::kOneInU, market.GetPriceU(kTestGood1));

  market.RegisterGood(kTestGood1);
  EXPECT_EQ(micro::kOneInU, market.GetPriceU(kTestGood1));

  // No bids or offer, price should be unchanged.
  market.FindPrices();
  EXPECT_EQ(micro::kOneInU, market.GetPriceU(kTestGood1));

  Quantity bid = MakeQuantity(kTestGood1, micro::kOneInU);
  Container bidder;
  EXPECT_EQ(0, market.TryToBuy(bid, &bidder));

  Quantity offer = MakeQuantity(kTestGood1, micro::kOneInU);
  Container seller;
  EXPECT_EQ(0, market.TryToSell(offer, &seller));

  // Bids exactly match offers, but are not effectual. No change.
  EXPECT_EQ(0, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_EQ(micro::kOneInU, market.GetPriceU(kTestGood1));

  seller += offer;
  // Seller's goods go into the warehouse in exchange for credit.
  EXPECT_EQ(micro::kOneInU, market.TryToSell(offer, &seller));
  EXPECT_EQ(micro::kOneInU, GetAmount(seller, kCredit));
  EXPECT_EQ(micro::kOneInU, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_EQ(micro::kOneInU, GetAmount(market.Proto()->warehouse(), kTestGood1));
  EXPECT_EQ(0, GetAmount(seller, kTestGood1));

  // Buyer can buy the goods, going into debt.
  EXPECT_EQ(micro::kOneInU, market.TryToBuy(bid, &bidder));
  EXPECT_EQ(micro::kOneInU, GetAmount(bidder, kDebt));
  EXPECT_EQ(micro::kOneInU, GetAmount(bidder, kTestGood1));
  EXPECT_EQ(0, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_EQ(0, GetAmount(market.Proto()->warehouse(), kTestGood1));

  // Bids match offers and went through. No change.
  EXPECT_EQ(micro::kOneInU, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_EQ(micro::kOneInU, market.GetPriceU(kTestGood1));

  // Try to buy twice.
  Add(kTestGood1, micro::kOneInU, &seller);
  Add(kSilver, micro::kOneInU, &bidder);
  EXPECT_EQ(micro::kOneInU, market.TryToSell(offer, &seller));
  EXPECT_EQ(micro::kOneInU, market.TryToBuy(bid, &bidder));
  EXPECT_EQ(0, market.TryToBuy(bid, &bidder));
  EXPECT_EQ(micro::kOneInU, market.GetVolume(kTestGood1));
  EXPECT_EQ(0, GetAmount(bidder, kSilver));
  EXPECT_EQ(micro::kOneInU, GetAmount(market.Proto()->warehouse(), kSilver));
  EXPECT_EQ(2 * micro::kOneInU, GetAmount(seller, kCredit));
  EXPECT_EQ(2 * micro::kOneInU, GetAmount(bidder, kTestGood1));
  EXPECT_EQ(0, GetAmount(seller, kTestGood1));
  EXPECT_EQ(micro::kOneInU, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_EQ(0, GetAmount(market.Proto()->warehouse(), kTestGood1));
  // More bids than offers; price rises.
  market.FindPrices();
  EXPECT_EQ(1250000, market.GetPriceU(kTestGood1));

  // Sell twice.
  SetAmount(kTestGood1, micro::kOneInU, market.Proto()->mutable_prices_u());
  SetAmount(kTestGood1, micro::kOneInU, &seller);
  EXPECT_EQ(0 * micro::kOneInU, market.TryToBuy(bid, &bidder));
  EXPECT_EQ(1 * micro::kOneInU, market.TryToSell(offer, &seller));
  EXPECT_EQ(0 * micro::kOneInU, market.TryToSell(offer, &seller));
  EXPECT_EQ(1 * micro::kOneInU, market.GetVolume(kTestGood1));
  // More offers than bids, but not effectual. No change.
  market.FindPrices();
  EXPECT_EQ(1 * micro::kOneInU, market.GetPriceU(kTestGood1));
  EXPECT_EQ(3 * micro::kOneInU, GetAmount(seller, kCredit));
  EXPECT_EQ(3 * micro::kOneInU, GetAmount(bidder, kTestGood1));
  EXPECT_EQ(0 * micro::kOneInU, GetAmount(seller, kTestGood1));
  EXPECT_EQ(1 * micro::kOneInU, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_EQ(0 * micro::kOneInU, GetAmount(market.Proto()->warehouse(), kTestGood1));
  EXPECT_EQ(2 * micro::kOneInU, GetAmount(bidder, kDebt));

  // Buy twice, now with money.
  SetAmount(kTestGood1, 1 * micro::kOneInU, market.Proto()->mutable_prices_u());
  SetAmount(kTestGood1, 1 * micro::kOneInU, &seller);
  SetAmount(kSilver, 2 * micro::kOneInU, &bidder);
  EXPECT_EQ(0 * micro::kOneInU, market.TryToBuy(bid, &bidder));
  EXPECT_EQ(1 * micro::kOneInU, market.TryToSell(offer, &seller));
  EXPECT_EQ(0 * micro::kOneInU, market.TryToBuy(bid, &bidder));
  EXPECT_EQ(1 * micro::kOneInU, market.GetVolume(kTestGood1));
  // More bids than offers, and they are effectual. Price should rise.
  market.FindPrices();
  EXPECT_EQ(1250000, market.GetPriceU(kTestGood1));
  EXPECT_EQ(3 * micro::kOneInU, GetAmount(seller, kCredit));
  EXPECT_EQ(1 * micro::kOneInU, GetAmount(seller, kSilver));
  EXPECT_EQ(1 * micro::kOneInU, GetAmount(bidder, kSilver));
  EXPECT_EQ(4 * micro::kOneInU, GetAmount(bidder, kTestGood1));
  EXPECT_EQ(0 * micro::kOneInU, GetAmount(seller, kTestGood1));
  EXPECT_EQ(1 * micro::kOneInU, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_EQ(0 * micro::kOneInU, GetAmount(market.Proto()->warehouse(), kTestGood1));
  EXPECT_EQ(2 * micro::kOneInU, GetAmount(bidder, kDebt));

  // Sell twice, now with goods.
  SetAmount(kTestGood1, 1 * micro::kOneInU, market.Proto()->mutable_prices_u());
  SetAmount(kTestGood1, 2 * micro::kOneInU, &seller);
  market.TryToSell(offer, &seller);
  market.TryToBuy(bid, &bidder);
  market.TryToSell(offer, &seller);

  // More offers than bids, and they are effectual. Price should fall.
  EXPECT_EQ(1 * micro::kOneInU, market.GetVolume(kTestGood1));
  market.FindPrices();
  EXPECT_EQ(800000, market.GetPriceU(kTestGood1));
  EXPECT_EQ(3 * micro::kOneInU, GetAmount(seller, kCredit));
  EXPECT_EQ(3 * micro::kOneInU, GetAmount(seller, kSilver));
  EXPECT_EQ(0 * micro::kOneInU, GetAmount(bidder, kSilver));
  EXPECT_EQ(5 * micro::kOneInU, GetAmount(bidder, kTestGood1));
  EXPECT_EQ(0 * micro::kOneInU, GetAmount(seller, kTestGood1));
  EXPECT_EQ(1 * micro::kOneInU, GetAmount(market.Proto()->market_debt(), kTestGood1));
  EXPECT_EQ(0 * micro::kOneInU, GetAmount(market.Proto()->warehouse(), kSilver));
  EXPECT_EQ(1 * micro::kOneInU, GetAmount(market.Proto()->warehouse(), kTestGood1));
  EXPECT_EQ(2 * micro::kOneInU, GetAmount(bidder, kDebt));
}

TEST_F(MarketTest, TransferMoney) {
  Market market;
  market.Proto()->set_name(kMarketName);
  market.Proto()->set_legal_tender(kSilver);
  market.Proto()->set_credit_limit(micro::kHundredInU);
  Container rich;
  Container poor;

  SetAmount(kSilver, 10, &rich);
  market.TransferMoney(10, &rich, &poor);
  EXPECT_EQ(GetAmount(rich, kSilver), 0);
  EXPECT_EQ(GetAmount(poor, kSilver), 10);

  SetAmount(kCredit, 10, &rich);
  market.TransferMoney(10, &rich, &poor);
  EXPECT_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_EQ(GetAmount(poor, kCredit), 10);

  SetAmount(kCredit, 10, &rich);
  SetAmount(kCredit, 0, &poor);
  SetAmount(kDebt, 5, &poor);
  market.TransferMoney(10, &rich, &poor);
  EXPECT_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_EQ(GetAmount(poor, kCredit), 5);
  EXPECT_EQ(GetAmount(poor, kDebt), 0);

  SetAmount(kDebt, 10, &poor);
  market.TransferMoney(10, &rich, &poor);
  EXPECT_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_EQ(GetAmount(rich, kDebt), 10);
  EXPECT_EQ(GetAmount(poor, kCredit), 5);
  EXPECT_EQ(GetAmount(poor, kDebt), 0);

  market.TransferMoney(10, &rich, &poor);
  EXPECT_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_EQ(GetAmount(rich, kDebt), 20);
  EXPECT_EQ(GetAmount(poor, kCredit), 15);
  EXPECT_EQ(GetAmount(poor, kDebt), 0);

  Clear(&rich);
  Clear(&poor);
  SetAmount(kCredit, 10, &rich);
  SetAmount(kSilver, 10, &rich);
  SetAmount(kDebt, 10, &poor);
  market.TransferMoney(40, &rich, &poor);
  EXPECT_EQ(GetAmount(rich, kCredit), 0);
  EXPECT_EQ(GetAmount(rich, kSilver), 0);
  EXPECT_EQ(GetAmount(rich, kDebt), 20);
  EXPECT_EQ(GetAmount(poor, kCredit), 20);
  EXPECT_EQ(GetAmount(poor, kSilver), 10);
  EXPECT_EQ(GetAmount(poor, kDebt), 0);
}

TEST_F(MarketTest, Available) {
  Market market;
  market.Proto()->set_legal_tender(kSilver);
  market.Proto()->set_name(kMarketName);
  market.Proto()->set_credit_limit(micro::kHundredInU);
  market.RegisterGood(kTestGood1);
  market.RegisterGood(kTestGood2);
  Container seller;
  const int64 kHalf = 500000;
  SetAmount(kTestGood1, kHalf, &seller);
  SetAmount(kTestGood1, micro::kOneInU, market.Proto()->mutable_prices_u());

  Quantity offer = MakeQuantity(kTestGood1, kHalf);
  EXPECT_EQ(kHalf, market.TryToSell(offer, &seller));
  EXPECT_EQ(kHalf, market.AvailableImmediately(kTestGood1));
  EXPECT_EQ(kHalf, GetAmount(seller, kCredit));
  EXPECT_EQ(kHalf, GetAmount(market.Proto()->market_debt(), kTestGood1));

  EXPECT_EQ(0, market.TryToSell(offer, &seller));
  EXPECT_EQ(kHalf, market.AvailableImmediately(kTestGood1));
  EXPECT_EQ(kHalf, GetAmount(seller, kCredit));
  EXPECT_EQ(kHalf, GetAmount(market.Proto()->market_debt(), kTestGood1));
}

TEST_F(MarketTest, BuySellBuy) {
  Market market;
  market.Proto()->set_legal_tender(kSilver);
  market.Proto()->set_name(kMarketName);
  market.Proto()->set_credit_limit(0);
  market.RegisterGood(kTestGood1);
  SetAmount(kTestGood1, micro::kOneInU, market.Proto()->mutable_prices_u());

  Container buyer;
  Container seller;
  SetAmount(kTestGood1, 2 * micro::kOneInU, &seller);

  Quantity offer = MakeQuantity(kTestGood1, micro::kOneInU);
  // Offer to buy, no goods available.
  EXPECT_EQ(0, market.TryToBuy(offer, &buyer));
  // Offer to sell, buyer has no money.
  SetAmount(kSilver, micro::kOneInU, market.Proto()->mutable_warehouse());
  EXPECT_EQ(1 * micro::kOneInU, market.TryToSell(offer, &seller));
  // Give buyer some money.
  SetAmount(kSilver, micro::kOneInU, &buyer);
  EXPECT_EQ(1 * micro::kOneInU, market.TryToBuy(offer, &buyer));

  // Create some additional supply.
  SetAmount(kSilver, micro::kOneInU, market.Proto()->mutable_warehouse());
  EXPECT_EQ(1 * micro::kOneInU, market.TryToSell(offer, &seller));
  // Buy offer should have disappeared, so price should drop.
  market.FindPrices();
  EXPECT_EQ(800000, market.GetPriceU(kTestGood1));
}

TEST_F(MarketTest, DecayGoods) {
  Market market;
  Container decay_rates_u;
  const int64 kNineTenths = 9 * micro::kOneInU / 10;
  market.RegisterGood(kTestGood1);
  SetAmount(kTestGood1, micro::kOneInU, market.Proto()->mutable_warehouse());
  EXPECT_EQ(micro::kOneInU, market.AvailableImmediately(kTestGood1));
  SetAmount(kTestGood1, kNineTenths, &decay_rates_u);
  market.DecayGoods(decay_rates_u);
  EXPECT_EQ(kNineTenths, market.AvailableImmediately(kTestGood1));
}

TEST_F(MarketTest, CanBuy) {
  SetPrice(kTestGood1, micro::kOneInU);
  market_.Proto()->set_credit_limit(0);
  market_.RegisterGood(kTestGood1);
  SetAmount(kTestGood1, micro::kOneInU, market_.Proto()->mutable_warehouse());

  Container basket;
  SetAmount(kTestGood1, micro::kOneInU, &basket);

  // Buyer has neither money nor credit.
  EXPECT_FALSE(market_.CanBuy(basket, buyer_));

  SetAmount(kSilver, micro::kOneInU, &buyer_);
  // Buyer now has money.
  EXPECT_TRUE(market_.CanBuy(basket, buyer_));

  SetAmount(kSilver, 0, &buyer_);
  market_.Proto()->set_credit_limit(micro::kTenInU);
  // Buyer now has credit.
  EXPECT_TRUE(market_.CanBuy(basket, buyer_));
  
  SetAmount(kTestGood2, micro::kOneInU, &basket);
  // Cannot buy what the market does not trade in.
  EXPECT_FALSE(market_.CanBuy(basket, buyer_));

  SetAmount(kTestGood2, micro::kOneInU, market_.Proto()->mutable_warehouse());
  // Not even when it's actually in the warehouse! (This should never happen.)
  EXPECT_FALSE(market_.CanBuy(basket, buyer_));

  market_.RegisterGood(kTestGood2);
  // Oh right, you mean _kTestGood2_! Yes sir, I remember now, we have some on
  // hand.
  EXPECT_TRUE(market_.CanBuy(basket, buyer_));
}

// Test that a net flow of zero doesn't affect price even with some stored in
// warehouse.
TEST_F(MarketTest, WarehouseVolume) {
  market_.RegisterGood(kTestGood1);
  SetPrice(kTestGood1, micro::kOneInU);
  SetAmount(kTestGood1, micro::kOneInU, market_.Proto()->mutable_warehouse());

  EXPECT_EQ(micro::kOneInU,
            market_.TryToBuy(kTestGood1, micro::kOneInU, &buyer_));
  Container seller;
  SetAmount(kTestGood1, micro::kOneInU, &seller);
  EXPECT_EQ(micro::kOneInU, market_.TryToSell(kTestGood1, micro::kOneInU, &seller));
  EXPECT_EQ(micro::kOneInU, GetAmount(market_.Proto()->warehouse(), kTestGood1));
  market_.FindPrices();
  EXPECT_EQ(micro::kOneInU, market_.GetPriceU(kTestGood1));
}

} // namespace market
