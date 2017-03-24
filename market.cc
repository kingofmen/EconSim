#include "market.h"

#include "goods_utils.h"

namespace market {

using market::proto::Quantity;
using market::proto::Container;

Market::Market(const market::proto::MarketProto &proto) {
  *mutable_prices() = proto.goods();
  *mutable_bids() = proto.bids();
  *mutable_offers() = proto.offers();
  *mutable_goods() = proto.goods();
  *mutable_volume() = proto.volume();
}

void Market::registerGood(const std::string &name) {
  if (Contains(goods(), name)) {
    return;
  }
  *mutable_goods() << name;
  *mutable_volume() << name;
  *mutable_prices() << name;
  *mutable_bids() << name;
  *mutable_offers() << name;

  mutable_prices()->mutable_quantities()->at(name).set_amount(1);
}

void Market::findPrices() {
  auto &volume = *mutable_volume()->mutable_quantities();
  auto &prices = *mutable_prices()->mutable_quantities();
  for (const auto &good : goods().quantities()) {
    const std::string &name = good.first;
    double bid = bids().quantities().at(name).amount();
    double offer = offers().quantities().at(name).amount();
    volume[name].set_amount(std::min(bid, offer));
    double ratio = bid / std::max(offer, 0.01);
    ratio = std::min(ratio, 1.25);
    ratio = std::max(ratio, 0.75);
    double price = prices[name].amount();
    price *= ratio;
    prices[name].set_amount(price);
  }
}

void Market::registerBid(const Quantity &bid) {
  if (!Contains(goods(), bid)) {
    return;
  }
  *mutable_bids() += bid;
}

void Market::registerOffer(const Quantity &offer) {
  if (!Contains(goods(), offer)) {
    return;
  }
  *mutable_offers() += offer;
}

double Market::getPrice(const std::string &name) const {
  if (!Contains(goods(), name)) {
    return -1;
  }

  return prices().quantities().at(name).amount();
}

double Market::getVolume(const std::string &name) const {
  if (!Contains(goods(), name)) {
    return -1;
  }

  return volume().quantities().at(name).amount();
}

} // namespace market
