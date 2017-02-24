// Class to hold prices and calculate new ones.

#ifndef MARKET_H
#define MARKET_H

#include <vector>
#include <string>

#include "proto/goods.pb.h"

namespace market {

class Market {
 public:
  Market() = default;
  ~Market() = default;

  // Balances current bids and offers to find new prices. Surplus offers cause
  // the price to go down, unmatched bids cause it to go up.
  void findPrices();

  // Register a bid or offer at the current price.
  void registerBid(const Quantity& bid);
  void registerOffer(const Quantity& offer);

  // Register a trade good to be traded in this market.
  void registerGood(const std::string& name);

  // Returns the price of the named good.
  double getPrice(const std::string& name) const;

  // Returns the amount of the named good that was traded.
  double getVolume(const std::string& name) const;

 private:
  // Current prices.
  Container prices_;

  // Bid amounts at current prices.
  Container bids_;

  // Offers at current prices.
  Container offers_;

  // Goods that this market trades in.
  Container goods_;

  // Amount of each good traded.
  Container volume_;
};

} // namespace market

#endif
