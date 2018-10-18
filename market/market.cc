#include "market.h"

#include <algorithm>

#include "market/goods_utils.h"
#include "util/arithmetic/microunits.h"

namespace market {

using market::proto::Quantity;
using market::proto::Container;
constexpr double kMaxPriceChange = micro::kOneInU / 4;

Measure Market::AvailableImmediately(const std::string& name) const {
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
  *proto_.mutable_prices_u() << name;
  SetAmount(name, micro::kOneInU, proto_.mutable_prices_u());
}

bool Market::BuyBasket(const proto::Container& basket,
                       proto::Container* target) {
  bool success = true;
  for (const auto& need : basket.quantities()) {
    if (need.second <= 0) {
      continue;
    }
    auto bought = TryToBuy(need.first, need.second, target);
    if (bought < need.second) {
      success = false;
    }
  }
  return success;
}

void Market::DecayGoods(const market::proto::Container& decay_rates_u) {
  micro::MultiplyU(*proto_.mutable_warehouse(), decay_rates_u);
}

void Market::FindPrices() {
  for (const auto& good : proto_.goods().quantities()) {
    const std::string& name = good.first;
    Measure matched = GetAmount(proto_.volume(), name);
    Measure bid = matched;
    for (const auto& buy : buy_offers_[name]) {
      // Effectual demand, ie demand backed up by money or credit.
      bid += std::min(buy.amount,
                      GetAmount(*buy.target, proto_.legal_tender()) +
                          MaxCredit(*buy.target));
    }
    Measure offer = GetAmount(proto_.warehouse(), name) + matched;
    if (std::min(bid, offer) < 1) {
      continue;
    }
    Measure ratio_u = micro::DivideU(std::max(bid, offer), std::min(bid, offer));
    ratio_u = std::min<Measure>(ratio_u, micro::kOneInU + kMaxPriceChange);
    if (offer > bid) {
      ratio_u = micro::DivideU(micro::kOneInU, ratio_u);
    }
    SetAmount(name,
              micro::MultiplyU(GetAmount(proto_.prices_u(), name), ratio_u),
              proto_.mutable_prices_u());
    SetAmount(name, 0, proto_.mutable_volume());
  }
  buy_offers_.clear();
}

Measure Market::MaxCredit(const Container& borrower) const {
  return proto_.credit_limit() - GetAmount(borrower, debt_token());
}

Measure Market::MaxMoney(const Container& buyer) const {
  return GetAmount(buyer, credit_token()) +
         GetAmount(buyer, proto_.legal_tender()) + MaxCredit(buyer);
}

void CancelDebt(const std::string& credit_token, const std::string& debt_token,
                Container* debtor) {
  Measure credit = GetAmount(*debtor, credit_token);
  Measure debt = GetAmount(*debtor, debt_token);
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

void Market::TransferMoney(Measure amount, Container* from,
                           Container* to) const {
  Measure credit = GetAmount(*from, credit_token());
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

Measure Market::TryToBuy(const Quantity& bid, Container* recipient) {
  return TryToBuy(bid.kind(), bid.amount(), recipient);
}

Measure Market::TryToBuy(const std::string& name, const Measure amount,
                         Container* recipient) {
  Measure amount_bought = std::min(amount, GetAmount(proto_.warehouse(), name));
  Measure price_u = GetPriceU(name);
  Measure max_money = MaxMoney(*recipient);
  if (micro::MultiplyU(price_u, amount_bought) > max_money) {
    amount_bought = micro::DivideU(max_money, price_u);
  }
  if (amount_bought > 0) {
    SetAmount(debt_token(), GetAmount(proto_.market_debt(), name),
              proto_.mutable_warehouse());
    SetAmount(name, 0, proto_.mutable_market_debt());

    Quantity transfer;
    transfer.set_kind(name);
    transfer.set_amount(amount_bought);
    Move(transfer, proto_.mutable_warehouse(), recipient);
    TransferMoney(GetPriceU(transfer), recipient, proto_.mutable_warehouse());
    *proto_.mutable_volume() += transfer;

    SetAmount(name, GetAmount(proto_.warehouse(), debt_token()),
              proto_.mutable_market_debt());
    SetAmount(debt_token(), 0, proto_.mutable_warehouse());
  }

  Measure amount_to_request = amount - amount_bought;
  auto& offers = buy_offers_[name];
  auto buy_offer = std::find_if(
      offers.begin(), offers.end(),
      [recipient](const Offer& offer) { return offer.target == recipient; });

  if (amount_to_request > 0) {
    if (buy_offer == offers.end()) {
      offers.emplace_back(amount_to_request, recipient);
    } else {
      buy_offer->amount = amount_to_request;
    }
  } else if (buy_offer != offers.end()) {
    offers.erase(buy_offer);
  }

  return amount_bought;
}

Measure Market::TryToSell(const Quantity& offer, Container* source) {
  if (!TradesIn(offer.kind())) {
    return 0;
  }

  const Measure amount_to_sell =
      std::min(offer.amount(), GetAmount(*source, offer));
  Measure amount_sold = 0;
  const std::string name = offer.kind();
  auto& buyers = buy_offers_[name];
  Measure unit_price_u = GetPriceU(name);

  std::vector<Offer> future_buyers;
  Quantity transfer = MakeQuantity(name, 0);
  for (auto& buyer : buyers) {
    transfer.set_amount(std::min(buyer.amount, amount_to_sell - amount_sold));
    if (transfer.amount() > 0) {
      Measure max_money = MaxMoney(*buyer.target);
      transfer.set_amount(
          std::min(transfer.amount(), micro::DivideU(max_money, unit_price_u)));

      Move(transfer, source, buyer.target);
      buyer.amount -= transfer.amount();
      TransferMoney(micro::MultiplyU(transfer.amount(), unit_price_u),
                    buyer.target, source);
      amount_sold += transfer.amount();
      *proto_.mutable_volume() += transfer;
    }

    if (buyer.amount > 0) {
      future_buyers.emplace_back(buyer.amount, buyer.target);
    }
  }

  buyers.swap(future_buyers);
  if (amount_sold >= amount_to_sell) {
    return amount_sold;
  }

  Quantity warehoused = MakeQuantity(name, amount_to_sell - amount_sold);
  Measure price_u = micro::MultiplyU(unit_price_u, warehoused.amount());
  Measure available = GetAmount(proto_.warehouse(), proto_.legal_tender()) +
                      proto_.credit_limit() -
                      GetAmount(proto_.market_debt(), warehoused);
  if (available < price_u) {
    price_u = available;
    Measure unit_price_u = GetPriceU(offer.kind());
    warehoused.set_amount(price_u / unit_price_u);
  }

  *source -= warehoused;
  *proto_.mutable_warehouse() += warehoused;

  TransferMoney(price_u, proto_.mutable_warehouse(), source);
  Quantity debt;
  debt.set_kind(debt_token());
  *proto_.mutable_warehouse() >> debt;
  Add(offer.kind(), debt.amount(), proto_.mutable_market_debt());

  return warehoused.amount() + amount_sold;
}

Measure Market::GetPriceU(const std::string& name, int turns) const {
  if (!TradesIn(name)) {
    return -1;
  }
  return GetAmount(proto_.prices_u(), name);
}

Measure Market::GetPriceU(const Quantity& quantity, int turns) const {
  if (!TradesIn(quantity.kind())) {
    return -1;
  }
  return micro::MultiplyU(GetAmount(proto_.prices_u(), quantity),
                          quantity.amount());
}

Measure Market::GetPriceU(const market::proto::Container& basket, int turns) const {
  Measure ret = 0;
  for (const auto& good : basket.quantities()) {
    Measure price = micro::MultiplyU(GetPriceU(good.first), good.second);
    if (price >= 0) {
      ret += price;
    }
  }
  return ret;
}

Measure Market::GetVolume(const std::string& name) const {
  if (!TradesIn(name)) {
    return -1;
  }
  return GetAmount(proto_.volume(), name);
}

} // namespace market
