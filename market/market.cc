#include "market.h"

#include "goods_utils.h"

namespace market {

using market::proto::Quantity;
using market::proto::Container;

double Market::AvailableToBuy(const std::string& name) const {
  const auto pos = sell_offers_.find(name);
  if (pos == sell_offers_.end()) {
    return 0;
  }
  double ret = 0;
  for (const auto& offer : pos->second) {
    ret += offer.good.amount();
  }
  return ret;
}

bool Market::AvailableToBuy(const Container& basket) const {
  for (const auto& good : basket.quantities()) {
    if (AvailableToBuy(good.first) < good.second) {
      return false;
    }
  }
  return true;
}

void Market::RegisterGood(const std::string& name) {
  if (Contains(goods(), name)) {
    return;
  }
  *mutable_goods() << name;
  *mutable_volume() << name;
  *mutable_prices() << name;
  SetAmount(name, 1, mutable_prices());
}

void Market::FindPrices() {
  for (const auto& good : goods().quantities()) {
    const std::string& name = good.first;
    double matched = GetAmount(volume(), name);
    double bid = 0;
    for (const auto& buy : buy_offers_[name]) {
      // Effectual demand, ie demand backed up by money or credit.
      bid += std::min(buy.good.amount(),
                      GetAmount(*buy.target, legal_tender()) +
                          MaxCredit(*buy.target));
    }
    double offer = 0;
    for (const auto& sell : sell_offers_[name]) {
      // Use the effectual supply, that is, goods offered for sale that actually
      // exist.
      offer += std::min(sell.good.amount(), GetAmount(*sell.target, name));
    }
    offer += matched;
    bid += matched;
    if (std::min(offer, bid) < 0.001) {
      continue;
    }
    double ratio = bid / std::max(offer, 0.01);
    ratio = std::min(ratio, 1.25);
    ratio = std::max(ratio, 0.75);
    SetAmount(name, GetAmount(prices(), name) * ratio, mutable_prices());
    SetAmount(name, 0, mutable_volume());
  }
  buy_offers_.clear();
  sell_offers_.clear();
}

double Market::MaxCredit(const Container& borrower) const {
  return credit_limit() - GetAmount(borrower, debt_token());
}

double Market::MaxMoney(const Container& buyer) const {
  return GetAmount(buyer, credit_token()) + GetAmount(buyer, legal_tender()) +
         MaxCredit(buyer);
}

void Market::RegisterBid(const Quantity& bid, Container* buyer) {
  if (!Contains(goods(), bid)) {
    return;
  }
  bool found = false;
  for (auto& offer : buy_offers_[bid.kind()]) {
    if (offer.target != buyer) {
      continue;
    }
    offer.good += bid.amount();
    found = true;
    break;
  }
  if (!found) {
    buy_offers_[bid.kind()].emplace_back(bid, buyer);
  }
}

void Market::RegisterOffer(const Quantity& bid, Container* seller) {
  if (!Contains(goods(), bid)) {
    return;
  }
  bool found = false;
  for (auto& offer : sell_offers_[bid.kind()]) {
    if (offer.target != seller) {
      continue;
    }
    offer.good += bid.amount();
    found = true;
    break;
  }
  if (!found) {
    sell_offers_[bid.kind()].emplace_back(bid, seller);
  }
}

void CancelDebt(const std::string& credit_token, const std::string& debt_token,
                Container* debtor) {
  double credit = GetAmount(*debtor, credit_token);
  double debt = GetAmount(*debtor, debt_token);
  if (credit >= debt) {
    SetAmount(debt_token, 0, debtor);
    SetAmount(credit_token, credit - debt, debtor);
  } else {
    SetAmount(credit_token, 0, debtor);
    SetAmount(debt_token, debt - credit, debtor);
  }
}

void Market::TransferMoney(double amount, Container* from,
                           Container* to) const {
  double credit = GetAmount(*from, credit_token());
  if (credit >= amount) {
    Move(credit_token(), amount, from, to);
    CancelDebt(credit_token(), debt_token(), to);
    return;
  } else {
    Move(credit_token(), credit, from, to);
    CancelDebt(credit_token(), debt_token(), to);
    amount -= credit;
  }

  credit = GetAmount(*from, legal_tender());
  if (credit >= amount) {
    Move(legal_tender(), amount, from, to);
    return;
  } else {
    Move(legal_tender(), credit, from, to);
    amount -= credit;
  }

  amount = std::min(MaxCredit(*from), amount);
  credit = GetAmount(*to, debt_token());
  if (credit >= amount) {
    Move(debt_token(), amount, to, from);
    return;
  } else {
    Move(debt_token(), credit, to, from);
    amount -= credit;
  }

  Add(debt_token(), amount, from);
  Add(credit_token(), amount, to);
}

double Market::TryToBuy(const Quantity& bid, Container* recipient) {
  return TryToBuy(bid.kind(), bid.amount(), recipient);
}

double Market::TryToBuy(const std::string name, const double amount, Container* recipient) {
  double amount_found = 0;
  auto& sellers = sell_offers_[name];
  double price = GetPrice(name);
  double max_money = GetAmount(*recipient, credit_token()) +
                     GetAmount(*recipient, legal_tender()) +
                     MaxCredit(*recipient);
  while (!sellers.empty()) {
    auto& seller = sellers.back();
    Quantity transfer = seller.good;
    transfer.set_amount(
        std::min(seller.good.amount(), amount - amount_found));
    transfer.set_amount(std::min(transfer.amount(), max_money / price));

    seller.good -= transfer.amount();
    Move(transfer, seller.target, recipient);
    if (seller.good.amount() < 1e-6) {
      sellers.pop_back();
    }
    double money_amount = transfer.amount() * price;
    TransferMoney(money_amount, recipient, seller.target);
    max_money -= money_amount;

    amount_found += transfer.amount();
    if (amount_found >= amount || max_money < 1e-6) {
      break;
    }
  }
  Add(name, amount_found, mutable_volume());
  for (auto& buyer : buy_offers_[name]) {
    if (buyer.target != recipient) {
      continue;
    }
    buyer.good -= amount_found;
    break;
  }
  return amount_found;
}

double Market::TryToSell(const Quantity& offer, Container* source) {
  double amount_found = 0;
  const std::string name = offer.kind();
  auto& buyers = buy_offers_[name];
  double price = GetPrice(name);
  while (!buyers.empty()) {
    auto& buyer = buyers.back();
    Quantity transfer = buyer.good;
    transfer.set_amount(std::min(transfer.amount(), offer.amount() - amount_found));
    double max_money = MaxMoney(*buyer.target);
    transfer.set_amount(std::min(transfer.amount(), max_money / price));

    Move(transfer, source, buyer.target);
    buyer.good -= transfer.amount();
    if (buyer.good.amount() < 1e-6) {
      buyers.pop_back();
    }
    TransferMoney(transfer.amount() * price, buyer.target, source);
    amount_found += transfer.amount();
    if (amount_found >= offer.amount()) {
      break;
    }
  }
  Add(name, amount_found, mutable_volume());
  for (auto& seller : sell_offers_[name]) {
    if (seller.target != source) {
      continue;
    }
    seller.good -= amount_found;
    break;
  }
  return amount_found;
}

double Market::GetPrice(const std::string& name) const {
  if (!Contains(goods(), name)) {
    return -1;
  }
  return GetAmount(prices(), name);
}

double Market::GetVolume(const std::string& name) const {
  if (!Contains(goods(), name)) {
    return -1;
  }
  return GetAmount(volume(), name);
}

} // namespace market
