// Utility functions for goods protos.
#include "games/market/goods_utils.h"

#include <limits>
#include <unordered_map>

#include "absl/strings/substitute.h"
#include "absl/strings/str_join.h"
#include "games/market/proto/goods.pb.h"
#include "util/arithmetic/microunits.h"

std::unordered_map<std::string, market::proto::TradeGood> goods_map_;
std::vector<std::string> goods_names_;

namespace market {

using market::proto::Quantity;
using market::proto::Container;

void ClearGoods() { goods_map_.clear(); }

void CreateTradeGood(const market::proto::TradeGood& good) {
  // TODO: Handle these errors.
  if (good.name() == "") {
    return;
  }
  if (goods_map_.find(good.name()) != goods_map_.end()) {
    return;
  }

  goods_map_[good.name()] = good;
  market::proto::TradeGood& copy = goods_map_[good.name()];
  if (copy.transport_type() != market::proto::TradeGood::TTT_IMMOBILE) {
    if (copy.bulk_u() < 1) {
      copy.set_bulk_u(1);
    }
    if (copy.weight_u() < 1) {
      copy.set_weight_u(1);
    }
  }

  goods_names_.push_back(good.name());
}

const std::vector<std::string>& ListGoods() { return goods_names_; }

bool Exists(const std::string& name) {
  return goods_map_.find(name) != goods_map_.end();
}
bool Exists(const market::proto::TradeGood& good) {
  return Exists(good.name());
}
bool AllGoodsExist(const market::proto::Container& con) {
  for (const auto& quantity : con.quantities()) {
    if (!Exists(quantity.first)) {
      return false;
    }
  }
  return true;
}

micro::Measure BulkU(const std::string& name) {
  return goods_map_[name].bulk_u();
}

micro::Measure DecayU(const std::string& name) {
  return goods_map_[name].decay_rate_u();
}

micro::Measure WeightU(const std::string& name) {
  return goods_map_[name].weight_u();
}

proto::TradeGood::TransportType TransportType(const std::string& name) {
  return goods_map_[name].transport_type();
}

void Add(const std::string& name, const micro::Measure amount, Container* con) {
  Quantity qua;
  qua.set_kind(name);
  qua.set_amount(amount);
  *con += qua;
}

void Clear(Container* con) { con->clear_quantities(); }

void CleanContainer(Container* con, micro::Measure tolerance) {
  std::vector<std::string> to_erase;
  for (const auto& quantity : con->quantities()) {
    if (quantity.second >= tolerance) {
      continue;
    }
    to_erase.emplace_back(quantity.first);
  }
  for (const auto& kind : to_erase) {
    con->mutable_quantities()->erase(kind);
  }
}

bool Contains(const Container& con, const std::string& name) {
  return con.quantities().find(name) != con.quantities().end();
}

bool Contains(const Container& con, const Quantity& qua) {
  return Contains(con, qua.kind());
}

void Copy(const Container& source, const Container& mask, Container* target) {
  for (const auto& m : mask.quantities()) {
    Copy(source, m.first, target);
  }
}

void Copy(const Container& source, const Quantity& mask, Container* target) {
  Copy(source, mask.kind(), target);
}

void Copy(const Container& source, const std::string& mask, Container* target) {
  Add(mask, GetAmount(source, mask), target);
}

void Erase(const std::string& kind, Container* con) {
  con->mutable_quantities()->erase(kind);
}

void Erase(const Quantity& qua, Container* con) {
  con->mutable_quantities()->erase(qua.kind());
}
void Erase(const std::pair<std::string, micro::Measure> amount,
           market::proto::Container* con) {
  Erase(amount.first, con);
}

bool Empty(const market::proto::Container& con) {
  for (const auto& good : con.quantities()) {
    if (good.second != 0) {
      return false;
    }
  }
  return true;
}

std::vector<Quantity> Expand(const market::proto::Container& con) {
  std::vector<Quantity> ret;
  for (const auto& good : con.quantities()) {
    ret.push_back(MakeQuantity(good.first, good.second));
  }

  // Ensure deterministic order of goods.
  std::sort(ret.begin(), ret.end(), [](const Quantity& one, const Quantity& two) {
      return one.kind() < two.kind();
    });
  return ret;
}

micro::Measure GetAmount(const Container& con, const std::string& name) {
  if (!Contains(con, name)) {
    return 0;
  }
  return con.quantities().at(name);
}

micro::Measure GetAmount(const Container& con, const Quantity& qua) {
  return GetAmount(con, qua.kind());
}

micro::Measure GetAmount(const Container& con,
                         const std::pair<std::string, micro::Measure>& qua) {
  return GetAmount(con, qua.first);
}

Quantity MakeQuantity(const std::string& name, const micro::Measure amount) {
  Quantity ret;
  ret.set_kind(name);
  ret.set_amount(amount);
  return ret;
}

void Move(const std::string& name, const micro::Measure amount, Container* from,
          Container* to) {
  Quantity transfer;
  transfer.set_kind(name);
  transfer.set_amount(amount);
  Move(transfer, from, to);
}

void Move(const Quantity& qua, Container* from, Container* to) {
  *from -= qua;
  *to += qua;
}

void SetAmount(const std::string& name, const micro::Measure amount,
               Container* con) {
  auto& quantities = *con->mutable_quantities();
  quantities[name] = amount;
}

void SetAmount(const Quantity& qua, Container* con) {
  SetAmount(qua.kind(), qua.amount(), con);
}

void SetAmount(const std::pair<std::string, micro::Measure> amount,
               market::proto::Container* con) {
  SetAmount(amount.first, amount.second, con);
}

market::proto::Container
SubtractFloor(const market::proto::Container& minuend,
              const market::proto::Container& subtrahend,
              micro::Measure floor) {
  auto diff = minuend;
  for (auto& quantity : subtrahend.quantities()) {
    auto existing = GetAmount(diff, quantity.first);
    auto subtract = quantity.second;
    if (existing <= floor) {
      continue;
    }
    if (subtract >= 0 &&
        std::numeric_limits<int64>::min() + subtract > existing) {
      SetAmount(quantity.first, floor, &diff);
      continue;
    } else if (subtract < 0 &&
               std::numeric_limits<int64>::max() + subtract < existing) {
      SetAmount(quantity.first, std::numeric_limits<int64>::max(), &diff);
      continue;
    }
    existing -= subtract;
    if (existing < floor) {
      existing = floor;
    }
    SetAmount(quantity.first, existing, &diff);
  }
  return diff;
}

void MultiplyU(proto::Container& lhs, int64 scale_u) {
  lhs *= scale_u;
  lhs /= micro::kOneInU;
}

void MultiplyU(market::proto::Container& lhs, const proto::Container& rhs_u) {
  lhs *= rhs_u;
  lhs /= micro::kOneInU;
}

std::string DisplayString(const std::string& kind, micro::Measure amount,
                          int digits) {
  return absl::Substitute("$0: $1", kind, micro::DisplayString(amount, digits));
}

std::string DisplayString(const proto::Quantity& q, int digits) {
  return DisplayString(q.kind(), q.amount(), digits);
}

std::string DisplayString(const proto::Container& q, int digits) {
  std::vector<std::string> items;
  for (auto& quantity : q.quantities()) {
    items.push_back(DisplayString(quantity.first, quantity.second, digits));
  }
  return absl::Substitute("($0)", absl::StrJoin(items, ", "));
}

namespace proto {

Container& operator<<(Container& con, Quantity& qua) {
  con += qua;
  qua.set_amount(0);
  return con;
}

Container& operator<<(Container& dst, Container& src) {
  dst += src;
  src.Clear();
  return dst;
}

Container& operator>>(Container& con, Quantity& qua) {
  if (!Contains(con, qua)) {
    return con;
  }
  qua.set_amount(qua.amount() + GetAmount(con, qua.kind()));
  con.mutable_quantities()->erase(qua.kind());
  return con;
}

Container& operator<<(Container& con, const std::string& name) {
  if (Contains(con, name)) {
    return con;
  }
  auto& quantities = *con.mutable_quantities();
  quantities[name] = 0;
  return con;
}

Quantity& operator+=(Quantity& lhs, const micro::Measure rhs) {
  lhs.set_amount(lhs.amount() + rhs);
  return lhs;
}

Quantity& operator*=(Quantity& lhs, const micro::Measure rhs) {
  lhs.set_amount(lhs.amount() * rhs);
  return lhs;
}

Quantity& operator/=(Quantity& lhs, const micro::Measure rhs) {
  lhs.set_amount(lhs.amount() / rhs);
  return lhs;
}

Quantity operator*(Quantity lhs, const micro::Measure rhs) {
  lhs *= rhs;
  return lhs;
}

Quantity operator/(Quantity lhs, const micro::Measure rhs) {
  lhs /= rhs;
  return lhs;
}

Quantity& operator-=(Quantity& lhs, const micro::Measure rhs) {
  lhs.set_amount(lhs.amount() - rhs);
  return lhs;
}

Container& operator+=(Container& lhs, const Container& rhs) {
  for (const auto& quantity : rhs.quantities()) {
    (*lhs.mutable_quantities())[quantity.first] += quantity.second;
  }
  return lhs;
}

Container& operator*=(Container& lhs, const micro::Measure rhs) {
  for (auto& quantity : *lhs.mutable_quantities()) {
    quantity.second *= rhs;
  }
  return lhs;
}

Container& operator/=(Container& lhs, const micro::Measure rhs) {
  for (auto& quantity : *lhs.mutable_quantities()) {
    quantity.second /= rhs;
  }
  return lhs;
}

Container& operator*=(Container& lhs, const Container& rhs) {
  auto& quantities = *lhs.mutable_quantities();
  for (auto& quantity : quantities) {
    quantity.second *= GetAmount(rhs, quantity.first);
  }
  return lhs;
}

Container& operator+=(Container& lhs, const Quantity& rhs) {
  auto& quantities = *lhs.mutable_quantities();
  quantities[rhs.kind()] += rhs.amount();
  return lhs;
}

Container& operator-=(Container& lhs, const Container& rhs) {
  for (const auto& quantity : rhs.quantities()) {
    (*lhs.mutable_quantities())[quantity.first] -= quantity.second;
  }
  return lhs;
}

Container& operator-=(Container& lhs, const Quantity& rhs) {
  auto& quantities = *lhs.mutable_quantities();
  if (!Contains(lhs, rhs)) {
    lhs << rhs.kind();
  }
  quantities[rhs.kind()] -= rhs.amount();
  return lhs;
}

Container operator+(Container lhs, const Container& rhs) {
  lhs += rhs;
  return lhs;
}

Container operator*(Container lhs, const micro::Measure rhs) {
  lhs *= rhs;
  return lhs;
}

Container operator/(Container lhs, const micro::Measure rhs) {
  lhs /= rhs;
  return lhs;
}

micro::Measure operator*(const Container& lhs, const Container& rhs) {
  micro::Measure ret = 0;
  for (const auto& quantity : rhs.quantities()) {
    ret += quantity.second * GetAmount(lhs, quantity.first);
  }
  return ret;
}

Container operator-(Container lhs, const Container& rhs) {
  lhs -= rhs;
  return lhs;
}

bool operator<(const Container& lhs, const Container& rhs) {
  // Always safe to subtract zero.
  if (lhs.quantities().empty()) {
    return true;
  }
  // Nothing can be subtracted from an empty container.
  if (rhs.quantities().empty()) {
    return false;
  }
  for (const auto& quantity : lhs.quantities()) {
    if (GetAmount(rhs, quantity.first) < quantity.second) {
      return false;
    }
  }
  return true;
}

bool operator>(const Container& lhs, const Container& rhs) { return rhs < lhs; }

bool operator<(const Container& lhs, const Quantity& rhs) {
  return GetAmount(lhs, rhs) < rhs.amount();
}

bool operator<=(const Container& lhs, const Quantity& rhs) {
  return GetAmount(lhs, rhs) <= rhs.amount();
}

bool operator>(const Container& lhs, const Quantity& rhs) {
  return GetAmount(lhs, rhs) > rhs.amount();
}

bool operator>=(const Container& lhs, const Quantity& rhs) {
  return GetAmount(lhs, rhs) >= rhs.amount();
}

} // namespace proto
} // namespace market
