#include "market.h"

#include "goods_utils.h"

namespace market {

void Market::registerGood(const std::string &name) {
  if (Contains(goods_, name)) {
    return;
  }
  goods_ << name;
  volume_ << name;
  prices_ << name;
  bids_ << name;
  offers_ << name;

  prices_.mutable_quantities()->at(name).set_amount(1);
}

void Market::findPrices() {
  auto &volume = *volume_.mutable_quantities();
  auto &prices = *prices_.mutable_quantities();
  for (const auto &good : goods_.quantities()) {
    const std::string &name = good.first;
    double bid = bids_.quantities().at(name).amount();
    double offer = offers_.quantities().at(name).amount();
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
  if (!Contains(goods_, bid)) {
    return;
  }
  bids_ += bid;
}

void Market::registerOffer(const Quantity &offer) {
  if (!Contains(goods_, offer)) {
    return;
  }
  offers_ += offer;
}

double Market::getPrice(const std::string &name) const {
  if (!Contains(goods_, name)) {
    return -1;
  }

  return prices_.quantities().at(name).amount();
}

double Market::getVolume(const std::string &name) const {
  if (!Contains(goods_, name)) {
    return -1;
  }

  return volume_.quantities().at(name).amount();
}

} // namespace market
