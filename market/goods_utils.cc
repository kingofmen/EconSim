// Utility functions for goods protos.
#include "goods_utils.h"

namespace market {

using market::proto::Quantity;
using market::proto::Container;

bool Contains(const Container& con, const std::string name) {
  return con.quantities().find(name) != con.quantities().end();
}

bool Contains(const Container& con, const Quantity& qua) {
  return Contains(con, qua.kind());
}

double GetAmount(const Container& con, const std::string name) {
  if (!Contains(con, name)) {
    return 0;
  }
  return con.quantities().at(name).amount();
}

double GetAmount(const Container& con, const Quantity& qua) {
  return GetAmount(con, qua.kind());
}

void Clear(Container& con) {
  auto& quantities = *con.mutable_quantities();
  for (auto& quantity : quantities) {
    quantity.second.set_amount(0);
  }
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
  auto& quantities = *con.mutable_quantities();
  quantities[qua.kind()].set_amount(0);
  return con;
}

Container& operator<<(Container& con, std::string name) {
  if (Contains(con, name)) {
    return con;
  }
  auto& quantities = *con.mutable_quantities();
  quantities[name].set_amount(0);
  return con;
}

Quantity &operator+=(Quantity &lhs, const double rhs) {
  lhs.set_amount(lhs.amount() + rhs);
  return lhs;
}

Quantity &operator*=(Quantity &lhs, const double rhs) {
  lhs.set_amount(lhs.amount() * rhs);
  return lhs;
}

Quantity operator*(Quantity lhs, const double rhs) {
  lhs *= rhs;
  return lhs;
}

Quantity &operator-=(Quantity &lhs, const double rhs) {
  lhs.set_amount(lhs.amount() - rhs);
  return lhs;
}

Container &operator+=(Container &lhs, const Container &rhs) {
  for (const auto &quantity : rhs.quantities()) {
    lhs += quantity.second;
  }
  return lhs;
}

Container &operator*=(Container &lhs, const double rhs) {
  for (auto &quantity : *lhs.mutable_quantities()) {
    quantity.second *= rhs;
  }
  return lhs;
}

Container &operator+=(Container &lhs, const Quantity &rhs) {
  auto &quantities = *lhs.mutable_quantities();
  if (Contains(lhs, rhs)) {
    quantities[rhs.kind()] += rhs.amount();
  } else {
    quantities[rhs.kind()] = rhs;
  }
  return lhs;
}

Container &operator-=(Container &lhs, const Container &rhs) {
  for (const auto &quantity : rhs.quantities()) {
    lhs -= quantity.second;
  }
  return lhs;
}

Container &operator-=(Container &lhs, const Quantity &rhs) {
  auto &quantities = *lhs.mutable_quantities();
  if (!Contains(lhs, rhs)) {
    lhs << rhs.kind();
  }
  quantities[rhs.kind()] -= rhs.amount();
  return lhs;
}

Container operator+(Container lhs, const Container &rhs) {
  lhs += rhs;
  return lhs;
}

Container operator*(Container lhs, const double rhs) {
  lhs *= rhs;
  return lhs;
}

double operator*(const market::proto::Container &lhs,
                 const market::proto::Container &rhs) {
  double ret = 0;
  for (const auto& quantity : rhs.quantities()) {
    ret += quantity.second.amount() * GetAmount(lhs, quantity.first);
  }
  return ret;
}

Container operator-(Container lhs, const Container &rhs) {
  lhs -= rhs;
  return lhs;
}

bool operator<(const Container &lhs, const Container &rhs) {
  if (rhs.quantities().empty()) {
    // Cannot be less than an empty container.
    return false;
  }
  for (const auto &quantity : rhs.quantities()) {
    if (lhs >= quantity.second) {
      return false;
    }
  }
  return true;
}

bool operator>(const Container &lhs, const Container &rhs) {
  if (lhs.quantities().empty()) {
    // Empty container is greater than nothing.
    return false;
  }
  for (const auto &quantity : rhs.quantities()) {
    if (lhs <= quantity.second) {
      return false;
    }
  }
  return true;
}

bool operator<=(const Container &lhs, const Container &rhs) {
  for (const auto &quantity : rhs.quantities()) {
    if (lhs > quantity.second) {
      return false;
    }
  }
  return true;
}

bool operator>=(const Container &lhs, const Container &rhs) {
  for (const auto &quantity : rhs.quantities()) {
    if (lhs < quantity.second) {
      return false;
    }
  }
  return true;
}

bool operator<(const Container &lhs, const Quantity &rhs) {
  return GetAmount(lhs, rhs) < rhs.amount();
}

bool operator<=(const Container &lhs, const Quantity &rhs) {
  return GetAmount(lhs, rhs) <= rhs.amount();
}

bool operator>(const Container &lhs, const Quantity &rhs) {
  return GetAmount(lhs, rhs) > rhs.amount();
}

bool operator>=(const Container &lhs, const Quantity &rhs) {
  return GetAmount(lhs, rhs) >= rhs.amount();
}

} // namespace proto
} // namespace market
