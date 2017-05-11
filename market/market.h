// Class to hold prices and calculate new ones.

#ifndef MARKET_H
#define MARKET_H

#include <map>
#include <string>
#include <vector>

#include "proto/goods.pb.h"
#include "proto/market.pb.h"

namespace market {

class Market : public market::proto::MarketProto {
public:
  Market() = default;
  Market(const market::proto::MarketProto& proto) : MarketProto(proto) {}
  ~Market() = default;

  // Returns the amount of name being offered.
  double AvailableToBuy(const std::string& name) const;

  // Balances current bids and offers to find new prices. Surplus offers cause
  // the price to go down, unmatched bids cause it to go up. Also clears
  // existing buy and sell offers since the prices change.
  void FindPrices();

  // Returns the maximum amount the supplicant can borrow.
  double MaxCredit(const market::proto::Container& borrower) const;

  // Register a bid or offer at the current price. This does not cause any goods
  // to change hands, it only registers the existence of a buyer or seller.
  void RegisterBid(const market::proto::Quantity& bid,
                   market::proto::Container* target);
  void RegisterOffer(const market::proto::Quantity& offer,
                     market::proto::Container* target);

  // Register a trade good to be traded in this market.
  void RegisterGood(const std::string& name);

  // Moves money, either legal tender or short-term credit, from from to to.
  void TransferMoney(double amount, market::proto::Container* from,
                     market::proto::Container* to) const;

  // Attempt to find a seller or buyer and actually make an exchange. Returns
  // the amount bought or sold.
  double TryToBuy(const market::proto::Quantity& bid,
                  market::proto::Container* recipient);
  double TryToSell(const market::proto::Quantity& offer,
                   market::proto::Container* source);

  // Returns the price of the named good.
  double GetPrice(const std::string& name) const;

  // Returns the amount of the named good that was traded.
  double GetVolume(const std::string& name) const;

 private:
  struct Offer {
    Offer(const market::proto::Quantity& offer, market::proto::Container* t)
        : good(offer), target(t) {}
    market::proto::Quantity good;
    market::proto::Container* target;
  };

  const std::string debt_token() const { return name() + "_short_term_debt"; }
  const std::string credit_token() const {
    return name() + "_short_term_credit";
  }

  std::unordered_map<std::string, std::vector<Offer>> buy_offers_;
  std::unordered_map<std::string, std::vector<Offer>> sell_offers_;
};

} // namespace market

#endif
