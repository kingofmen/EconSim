// Utility functions for goods protos.
#include "goods_utils.h"

namespace market {

using market::proto::Quantity;
using market::proto::Container;

void Add(const std::string& name, const Measure amount, Container* con) {
  Quantity qua;
  qua.set_kind(name);
  qua.set_amount(amount);
  *con += qua;
}

void Clear(Container* con) { con->clear_quantities(); }

void CleanContainer(Container* con, Measure tolerance) {
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

Measure GetAmount(const Container& con, const std::string& name) {
  if (!Contains(con, name)) {
    return 0;
  }
  return con.quantities().at(name);
}

Measure GetAmount(const Container& con, const Quantity& qua) {
  return GetAmount(con, qua.kind());
}

Quantity MakeQuantity(const std::string& name, const Measure amount) {
  Quantity ret;
  ret.set_kind(name);
  ret.set_amount(amount);
  return ret;
}

void Move(const std::string& name, const Measure amount, Container* from,
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

void SetAmount(const std::string& name, const Measure amount, Container* con) {
  auto& quantities = *con->mutable_quantities();
  quantities[name] = amount;
}

void SetAmount(const Quantity& qua, Container* con) {
  SetAmount(qua.kind(), qua.amount(), con);
}

namespace proto {

Container& operator<<(Container& con, Quantity& qua) {
  con += qua;
  qua.set_amount(0);
  return con;
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

Quantity& operator+=(Quantity& lhs, const Measure rhs) {
  lhs.set_amount(lhs.amount() + rhs);
  return lhs;
}

Quantity& operator*=(Quantity& lhs, const Measure rhs) {
  lhs.set_amount(lhs.amount() * rhs);
  return lhs;
}

Quantity operator*(Quantity lhs, const Measure rhs) {
  lhs *= rhs;
  return lhs;
}

Quantity& operator-=(Quantity& lhs, const Measure rhs) {
  lhs.set_amount(lhs.amount() - rhs);
  return lhs;
}

Container& operator+=(Container& lhs, const Container& rhs) {
  for (const auto& quantity : rhs.quantities()) {
    (*lhs.mutable_quantities())[quantity.first] += quantity.second;
  }
  return lhs;
}

Container& operator*=(Container& lhs, const Measure rhs) {
  for (auto& quantity : *lhs.mutable_quantities()) {
    quantity.second *= rhs;
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

Container operator*(Container lhs, const Measure rhs) {
  lhs *= rhs;
  return lhs;
}

Measure operator*(const Container& lhs, const Container& rhs) {
  Measure ret = 0;
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
