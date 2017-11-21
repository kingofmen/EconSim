// Class to hold prices and calculate new ones.

#ifndef MARKET_H
#define MARKET_H

#include <map>
#include <string>
#include <vector>

#include "market/proto/goods.pb.h"
#include "market/proto/market.pb.h"

namespace market {

typedef double Measure;

class Market {
public:
  Market() = default;
  Market(const proto::MarketProto& proto) : proto_(proto) {}
  ~Market() = default;

  // Returns the amount of name immediately available.
  Measure AvailableImmediately(const std::string& name) const;

  // True if the basket can be bought right away.
  bool AvailableImmediately(const market::proto::Container& basket) const;

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

  // Returns the price of the named good.
  Measure GetPrice(const std::string& name) const;

  // Returns the price of the given amount of the given good.
  Measure GetPrice(const market::proto::Quantity& quantity) const;

  // Returns the price of the goods in the basket.
  Measure GetPrice(const market::proto::Container& basket) const;

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
};

} // namespace market

#endif
