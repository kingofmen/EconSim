#include "market.h"

#include <algorithm>

#include "goods_utils.h"

namespace market {

using market::proto::Quantity;
using market::proto::Container;

double Market::AvailableImmediately(const std::string& name) const {
  return GetAmount(proto_.warehouse(), name);
}

bool Market::AvailableImmediately(const Container& basket) const {
  return proto_.warehouse() > basket;
}

void Market::RegisterGood(const std::string& name) {
  if (TradesIn(name)) {
    return;
  }
  *proto_.mutable_goods() << name;
  *proto_.mutable_volume() << name;
  *proto_.mutable_prices() << name;
  SetAmount(name, 1, proto_.mutable_prices());
}

void Market::FindPrices() {
  for (const auto& good : proto_.goods().quantities()) {
    const std::string& name = good.first;
    double matched = GetAmount(proto_.volume(), name);
    double bid = matched;
    for (const auto& buy : buy_offers_[name]) {
      // Effectual demand, ie demand backed up by money or credit.
      bid += std::min(buy.amount,
                      GetAmount(*buy.target, proto_.legal_tender()) +
                          MaxCredit(*buy.target));
    }
    double offer = GetAmount(proto_.warehouse(), name) + matched;
    if (std::min(offer, bid) < 0.001) {
      continue;
    }
    double ratio = bid / std::max(offer, 0.01);
    ratio = std::min(ratio, 1.25);
    ratio = std::max(ratio, 0.75);
    SetAmount(name, GetAmount(proto_.prices(), name) * ratio,
              proto_.mutable_prices());
    SetAmount(name, 0, proto_.mutable_volume());
  }
  buy_offers_.clear();
}

double Market::MaxCredit(const Container& borrower) const {
  return proto_.credit_limit() - GetAmount(borrower, debt_token());
}

double Market::MaxMoney(const Container& buyer) const {
  return GetAmount(buyer, credit_token()) +
         GetAmount(buyer, proto_.legal_tender()) + MaxCredit(buyer);
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

bool Market::TradesIn(const std::string& name) const {
  return Contains(proto_.goods(), name);
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

  credit = GetAmount(*from, proto_.legal_tender());
  if (credit >= amount) {
    Move(proto_.legal_tender(), amount, from, to);
    return;
  } else {
    Move(proto_.legal_tender(), credit, from, to);
    amount -= credit;
  }

  // TODO: Transfers that require exceeding the debt limit should be an error
  // status, or otherwise not silently ignored.
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

double Market::TryToBuy(const std::string& name, const double amount,
                        Container* recipient) {
  double amount_bought = std::min(amount, GetAmount(proto_.warehouse(), name));
  double price = GetPrice(name);
  double max_money = MaxMoney(*recipient);
  if (price * amount_bought > max_money) {
    amount_bought = max_money / price;
  }
  if (amount_bought > 0) {
    SetAmount(debt_token(), GetAmount(proto_.market_debt(), name),
              proto_.mutable_warehouse());
    SetAmount(name, 0, proto_.mutable_market_debt());

    Quantity transfer;
    transfer.set_kind(name);
    transfer.set_amount(amount_bought);
    Move(transfer, proto_.mutable_warehouse(), recipient);
    TransferMoney(GetPrice(transfer), recipient, proto_.mutable_warehouse());
    *proto_.mutable_volume() += transfer;

    SetAmount(name, GetAmount(proto_.warehouse(), debt_token()),
              proto_.mutable_market_debt());
    SetAmount(debt_token(), 0, proto_.mutable_warehouse());
  }

  double amount_to_request = amount - amount_bought;
  if (amount_to_request > 0) {
    auto& offers = buy_offers_[name];
    auto buy_offer = std::find_if(
        offers.begin(), offers.end(),
        [recipient](const Offer& offer) { return offer.target == recipient; });

    if (buy_offer == offers.end()) {
      offers.emplace_back(amount_to_request, recipient);
    } else {
      buy_offer->amount = amount_to_request;
    }
  }

  return amount_bought;
}

double Market::TryToSell(const Quantity& offer, Container* source) {
  if (!TradesIn(offer.kind())) {
    return 0;
  }

  double amount_to_sell = std::min(offer.amount(), GetAmount(*source, offer));
  double amount_sold = 0;
  const std::string name = offer.kind();
  auto& buyers = buy_offers_[name];
  double unit_price = GetPrice(name);
  while (!buyers.empty()) {
    auto& buyer = buyers.back();
    Quantity transfer = MakeQuantity(
        name, std::min(buyer.amount, amount_to_sell - amount_sold));
    double max_money = MaxMoney(*buyer.target);
    transfer.set_amount(std::min(transfer.amount(), max_money / unit_price));

    Move(transfer, source, buyer.target);
    buyer.amount -= transfer.amount();
    TransferMoney(transfer.amount() * unit_price, buyer.target, source);
    amount_sold += transfer.amount();
    *proto_.mutable_volume() += transfer;

    if (buyer.amount < 1e-6) {
      buyers.pop_back();
    }

    if (amount_sold >= amount_to_sell) {
      return amount_sold;
    }
  }
  Quantity warehoused = MakeQuantity(name, amount_to_sell - amount_sold);
  double price = unit_price * warehoused.amount();
  double available =
      proto_.credit_limit() - GetAmount(proto_.market_debt(), warehoused);
  if (available < price) {
    price = available;
    double unit_price = GetPrice(offer.kind());
    warehoused.set_amount(price / unit_price);
  }

  *source -= warehoused;
  *proto_.mutable_warehouse() += warehoused;

  TransferMoney(price, proto_.mutable_warehouse(), source);
  Quantity debt;
  debt.set_kind(debt_token());
  *proto_.mutable_warehouse() >> debt;
  Add(offer.kind(), debt.amount(), proto_.mutable_market_debt());

  return warehoused.amount() + amount_sold;
}

double Market::GetPrice(const std::string& name) const {
  if (!TradesIn(name)) {
    return -1;
  }
  return GetAmount(proto_.prices(), name);
}

double Market::GetPrice(const Quantity& quantity) const {
  if (!TradesIn(quantity.kind())) {
    return -1;
  }
  return GetAmount(proto_.prices(), quantity) * quantity.amount();
}

double Market::GetPrice(const market::proto::Container& basket) const {
  double ret = 0;
  for (const auto& good : basket.quantities()) {
    double price = GetPrice(good.first) * good.second;
    if (price >= 0) {
      ret += price;
    }
  }
  return ret;
}

double Market::GetVolume(const std::string& name) const {
  if (!TradesIn(name)) {
    return -1;
  }
  return GetAmount(proto_.volume(), name);
}

} // namespace market
