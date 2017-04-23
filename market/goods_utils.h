// Utility functions for manipulating goods.
#ifndef MARKET_GOODS_UTILS_H
#define MARKET_GOODS_UTILS_H

#include "proto/goods.pb.h"

namespace market {

// Sets all amounts in the container to 0.
void Clear(market::proto::Container* con);

// Removes goods with less than tolerance amount from con.
void CleanContainer(market::proto::Container* con, double tolerance = 0.000001);

// Returns true if con has an entry for the good name, even if the amount is
// zero.
bool Contains(const market::proto::Container &con, const std::string name);
// Returns true if con has an entry for the good in qua. The amount is ignored.
bool Contains(const market::proto::Container &con,
              const market::proto::Quantity &qua);

// Returns the amount of name in con; zero if not set.
double GetAmount(const market::proto::Container& con, const std::string name);
// Returns the amount of qua.kind() in con; zero if not set.
double GetAmount(const market::proto::Container& con,
                 const market::proto::Quantity& qua);

// Sets the amount of name in con.
void SetAmount(const std::string name, const double amount,
               market::proto::Container* con);
void SetAmount(const market::proto::Quantity& qua,
               market::proto::Container* con);

namespace proto {

// Create an entry for the good name in con. No effect if the entry exists
// already.
market::proto::Container &operator<<(market::proto::Container &con,
                                     std::string name);

// Adds qua to con, setting the amount in qua to zero.
market::proto::Container &operator<<(market::proto::Container &con,
                                     market::proto::Quantity &qua);

// Moves any qua.kind() in con into qua.
market::proto::Container &operator>>(market::proto::Container &con,
                                     market::proto::Quantity &qua);

market::proto::Quantity &operator+=(market::proto::Quantity &lhs,
                                    const double rhs);
market::proto::Quantity &operator-=(market::proto::Quantity &lhs,
                                    const double rhs);

market::proto::Quantity &operator*=(market::proto::Quantity &lhs,
                                    const double rhs);

market::proto::Quantity operator*(market::proto::Quantity lhs,
                                  const double rhs);

market::proto::Container &operator+=(market::proto::Container &lhs,
                                     const market::proto::Container &rhs);

market::proto::Container &operator+=(market::proto::Container &lhs,
                                     const market::proto::Quantity &rhs);
market::proto::Container &operator-=(market::proto::Container &lhs,
                                     const market::proto::Container &rhs);
market::proto::Container &operator-=(market::proto::Container &lhs,
                                     const market::proto::Quantity &rhs);
market::proto::Container &operator*=(market::proto::Container &lhs,
                                     const double rhs);
market::proto::Container operator+(market::proto::Container lhs,
                                   const market::proto::Container &rhs);
market::proto::Container operator-(market::proto::Container lhs,
                                   const market::proto::Container &rhs);
market::proto::Container operator*(market::proto::Container lhs,
                                   const double rhs);

// Matrix-vector product; the amount of each good in lhs is multiplied by the
// corresponding amount in rhs. Note that this is not the same as the
// dot-product operator*.
market::proto::Container& operator*=(market::proto::Container& lhs,
                                     const market::proto::Container& rhs);

// Dot product - for example, multiply a basket of goods by their prices.
double operator*(const market::proto::Container &lhs,
                 const market::proto::Container &rhs);

// Note that these are counterintuitive; they cannot be used for sorting. The
// intended meaning of a > b is that b may safely be subtracted from a, that is,
// there will be no negative entries after the subtraction. Likewise if a < b
// then (b - a) is safe. So we have the intuitive relation
// a < b  <=>  b > a,
// but it is not true that
// !(a < b) => b <= a.
// It is possible for both subtractions, a - b and b - a, to be unsafe; for
// example if the two vectors are orthogonal this is trivially true.
bool operator<(const market::proto::Container& lhs,
               const market::proto::Container& rhs);
bool operator>(const market::proto::Container &lhs,
               const market::proto::Container &rhs);
// The <= and >= operators are aliases for the < and > operators, as they all
// express safe subtraction rather than sizes per se.
inline bool operator<=(const market::proto::Container &lhs,
                const market::proto::Container &rhs) {
  return lhs < rhs;
}
inline bool operator>=(const market::proto::Container &lhs,
                const market::proto::Container &rhs) {
  return lhs > rhs;
}
bool operator<(const market::proto::Container &lhs,
               const market::proto::Quantity &rhs);
bool operator>(const market::proto::Container &lhs,
               const market::proto::Quantity &rhs);
bool operator<=(const market::proto::Container &lhs,
                const market::proto::Quantity &rhs);
bool operator>=(const market::proto::Container &lhs,
                const market::proto::Quantity &rhs);

} // namespace proto
} // namespace market

#endif
