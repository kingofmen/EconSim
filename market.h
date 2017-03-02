// Class to hold prices and calculate new ones.

#ifndef MARKET_H
#define MARKET_H

#include <string>
#include <vector>

#include "proto/goods.pb.h"
#include "proto/market.pb.h"

namespace market {

class Market {
public:
  Market(const market::proto::MarketProto& proto);
  ~Market() = default;

  // Balances current bids and offers to find new prices. Surplus offers cause
  // the price to go down, unmatched bids cause it to go up.
  void findPrices();

  // Register a bid or offer at the current price.
  void registerBid(const market::proto::Quantity &bid);
  void registerOffer(const market::proto::Quantity &offer);

  // Register a trade good to be traded in this market.
  void registerGood(const std::string &name);

  // Returns the price of the named good.
  double getPrice(const std::string &name) const;

  // Returns the amount of the named good that was traded.
  double getVolume(const std::string &name) const;

  // Copies the information into the provided protobuf.
  void ToProto(market::proto::MarketProto* proto) const;

 private:
  // Current prices.
  market::proto::Container prices_;

  // Bid amounts at current prices.
  market::proto::Container bids_;

  // Offers at current prices.
  market::proto::Container offers_;

  // Goods that this market trades in.
  market::proto::Container goods_;

  // Amount of each good traded.
  market::proto::Container volume_;
};

} // namespace market

#endif
