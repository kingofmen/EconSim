// Class to hold prices and calculate new ones.

#ifndef MARKET_H
#define MARKET_H

#include <map>
#include <string>
#include <vector>

#include "market/proto/goods.pb.h"
#include "market/proto/market.pb.h"
#include "util/headers/int_types.h"

namespace market {

typedef int64 Measure;

// Interface for estimating prices.
class PriceEstimator {
 public:
  // Returns the price of the named good, turns ahead.
  virtual Measure GetPriceU(const std::string& name, int turns) const = 0;

  // Returns the price of the given amount of the given good.
  virtual Measure GetPriceU(const market::proto::Quantity& quantity, int turns) const = 0;

  // Returns the price of the goods in the basket.
  virtual Measure GetPriceU(const market::proto::Container& basket, int turns) const = 0;
};

// Interface for estimating availability.
class AvailabilityEstimator {
 public:
  // Returns the amount of name immediately available.
  Measure AvailableImmediately(const std::string& name) const {
    return Available(name, 0);
  }

  // Returns true if the basket can be bought right away.
  bool AvailableImmediately(const market::proto::Container& basket) const {
    return Available(basket, 0);
  }

  virtual Measure Available(const std::string& name, int ahead) const = 0;
  virtual bool Available(const market::proto::Container& basket,
                         int ahead) const = 0;
};

// Price finder. Implements PriceEstimator, simply returning the current
// price, and AvailabilityEstimator, returning the current availability.
class Market : public PriceEstimator, public AvailabilityEstimator {
public:
  Market() = default;
  Market(const proto::MarketProto& proto) : proto_(proto) {}
  ~Market() = default;

  Measure Available(const std::string& name, int ahead = 0) const override;
  bool Available(const market::proto::Container& basket,
                 int ahead = 0) const override;

  // Attempts to buy each good in the provided basket from market, paying with
  // target. Returns true if all the buys are successful.
  bool BuyBasket(const proto::Container& basket, proto::Container* target);

  // Returns true if all the goods in basket are available and buyer can pay
  // for them.
  bool CanBuy(const proto::Container& basket,
              const proto::Container& buyer) const;

  // Make goods stored in warehouse decay at the given rates.
  void DecayGoods(const market::proto::Container& rates);

  // Balances current bids and offers to find new prices. Surplus offers cause
  // the price to go down, unmatched bids cause it to go up. Also clears
  // existing buy and sell offers since the prices change.
  void FindPrices();

  // Returns the maximum amount the supplicant can borrow.
  Measure MaxCredit(const market::proto::Container& borrower) const;

  // Returns the maximum the buyer can spend, including credit.
  Measure MaxMoney(const market::proto::Container& buyer) const;

  // Register a trade good to be traded in this market.
  void RegisterGood(const std::string& name);

  // Returns true if the market trades in the good.
  bool TradesIn(const std::string& name) const;

  // Moves money, either legal tender or short-term credit, from from to to.
  void TransferMoney(Measure amount, market::proto::Container* from,
                     market::proto::Container* to) const;

  // Buys goods from the warehouse if there are enough; otherwise registers a
  // request to buy. Returns the amount bought.
  Measure TryToBuy(const market::proto::Quantity& bid,
                   market::proto::Container* recipient);
  Measure TryToBuy(const std::string& name, const Measure amount,
                   market::proto::Container* recipient);

  // Finds a buyer and sells it the goods at the current price, if possible.
  // Otherwise places the offered goods in the warehouse if the market can pay
  // for them, in credit, legal tender, or newly-issued debt. Returns the amount
  // accepted.
  Measure TryToSell(const market::proto::Quantity& offer,
                    market::proto::Container* source);
  Measure TryToSell(const std::string& name, const Measure amount,
                    market::proto::Container* source);

  // Returns the price of the named good.
  Measure GetPriceU(const std::string& name, int turns = 0) const override;

  // Returns the price of the given amount of the given good.
  Measure GetPriceU(const market::proto::Quantity& quantity, int turns = 0) const override;

  // Returns the price of the goods in the basket.
  Measure GetPriceU(const market::proto::Container& basket, int turns = 0) const override;

  // Returns the amount of the named good that was traded.
  Measure GetVolume(const std::string& name) const;

  // The underlying protobuf.
  const proto::MarketProto& Proto() const { return proto_; }
  proto::MarketProto* Proto() { return &proto_; }

private:
  struct Offer {
    Offer(const Measure a, market::proto::Container* t)
        : amount(a), target(t) {}
    Measure amount;
    market::proto::Container* target;
  };

  const std::string debt_token() const {
    return proto_.name() + "_short_term_debt";
  }
  const std::string credit_token() const {
    return proto_.name() + "_short_term_credit";
  }

  std::unordered_map<std::string, std::vector<Offer>> buy_offers_;

  proto::MarketProto proto_;

  // Stores how much has flowed into or out of the warehouse this turn.
  proto::Container flow_tracker_;
};

} // namespace market

#endif
