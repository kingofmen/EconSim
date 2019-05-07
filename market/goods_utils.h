// Utility functions for manipulating goods.
#ifndef MARKET_GOODS_UTILS_H
#define MARKET_GOODS_UTILS_H

#include "market/proto/goods.pb.h"
#include "util/headers/int_types.h"

namespace market {
typedef int64 Measure;

// Registers a trade good.
void CreateTradeGood(const market::proto::TradeGood& good);
// Clears all good information.
void ClearGoods();

// Information about trade goods.
Measure BulkU(const std::string& name);
Measure DecayU(const std::string& name);
Measure WeightU(const std::string& name);
proto::TradeGood::TransportType TransportType(const std::string& name);

// Adds amount of name to con.
void Add(const std::string& name, const Measure amount,
         market::proto::Container* con);

// Sets all amounts in the container to 0.
void Clear(market::proto::Container* con);

// Removes goods with less than tolerance amount from con.
void CleanContainer(market::proto::Container* con, Measure tolerance = 1);

// Returns true if con has an entry for the good name, even if the amount is
// zero.
bool Contains(const market::proto::Container& con, const std::string& name);
// Returns true if con has an entry for the good in qua. The amount is ignored.
bool Contains(const market::proto::Container& con,
              const market::proto::Quantity& qua);

// Returns the amount of name in con; zero if not set.
Measure GetAmount(const market::proto::Container& con, const std::string& name);
// Returns the amount of qua.kind() in con; zero if not set.
Measure GetAmount(const market::proto::Container& con,
                  const market::proto::Quantity& qua);

// Returns a quantity with the given values.
market::proto::Quantity MakeQuantity(const std::string& name,
                                     const Measure amount);

// Moves amount of name from from to to.
void Move(const std::string& name, const Measure amount,
          market::proto::Container* from, market::proto::Container* to);
void Move(const market::proto::Quantity& qua, market::proto::Container* from,
          market::proto::Container* to);

// Sets the amount of name in con.
void SetAmount(const std::string& name, const Measure amount,
               market::proto::Container* con);
void SetAmount(const market::proto::Quantity& qua,
               market::proto::Container* con);

// Subtracts the subtrahend from the minuend, leaving at least floor of each
// quantity, unless the amount was already smaller than floor, in which case it
// is unchanged.
market::proto::Container
SubtractFloor(const market::proto::Container& minuend,
              const market::proto::Container& subtrahend,
              market::Measure floor = 0);

namespace proto {

// Create an entry for the good name in con. No effect if the entry exists
// already.
market::proto::Container& operator<<(market::proto::Container& con,
                                     const std::string& name);

// Adds qua to con, setting the amount in qua to zero.
market::proto::Container& operator<<(market::proto::Container& con,
                                     market::proto::Quantity& qua);

// Moves all goods in src into dst.
market::proto::Container& operator<<(market::proto::Container& dst,
                                     market::proto::Container& src);

// Moves any qua.kind() in con into qua.
market::proto::Container& operator>>(market::proto::Container& con,
                                     market::proto::Quantity& qua);

market::proto::Quantity& operator+=(market::proto::Quantity& lhs,
                                    const Measure rhs);
market::proto::Quantity& operator-=(market::proto::Quantity& lhs,
                                    const Measure rhs);
market::proto::Quantity& operator*=(market::proto::Quantity& lhs,
                                    const Measure rhs);
market::proto::Quantity& operator/=(market::proto::Quantity& lhs,
                                    const Measure rhs);

market::proto::Quantity operator*(market::proto::Quantity lhs,
                                  const Measure rhs);
market::proto::Quantity operator/(market::proto::Quantity lhs,
                                  const Measure rhs);

market::proto::Container& operator+=(market::proto::Container& lhs,
                                     const market::proto::Container& rhs);

market::proto::Container& operator+=(market::proto::Container& lhs,
                                     const market::proto::Quantity& rhs);
market::proto::Container& operator-=(market::proto::Container& lhs,
                                     const market::proto::Container& rhs);
market::proto::Container& operator-=(market::proto::Container& lhs,
                                     const market::proto::Quantity& rhs);
market::proto::Container& operator*=(market::proto::Container& lhs,
                                     const Measure rhs);
market::proto::Container& operator/=(market::proto::Container& lhs,
                                     const Measure rhs);
market::proto::Container operator+(market::proto::Container lhs,
                                   const market::proto::Container& rhs);
market::proto::Container operator-(market::proto::Container lhs,
                                   const market::proto::Container& rhs);
market::proto::Container operator*(market::proto::Container lhs,
                                   const Measure rhs);
market::proto::Container operator/(market::proto::Container lhs,
                                   const Measure rhs);

// Matrix-vector product; the amount of each good in lhs is multiplied by the
// corresponding amount in rhs. Note that this is not the same as the
// dot-product operator*.
market::proto::Container& operator*=(market::proto::Container& lhs,
                                     const market::proto::Container& rhs);

// Dot product - for example, multiply a basket of goods by their prices.
Measure operator*(const market::proto::Container& lhs,
                  const market::proto::Container& rhs);

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
bool operator>(const market::proto::Container& lhs,
               const market::proto::Container& rhs);
// The <= and >= operators are aliases for the < and > operators, as they all
// express safe subtraction rather than sizes per se.
inline bool operator<=(const market::proto::Container& lhs,
                       const market::proto::Container& rhs) {
  return lhs < rhs;
}
inline bool operator>=(const market::proto::Container& lhs,
                       const market::proto::Container& rhs) {
  return lhs > rhs;
}
bool operator<(const market::proto::Container& lhs,
               const market::proto::Quantity& rhs);
bool operator>(const market::proto::Container& lhs,
               const market::proto::Quantity& rhs);
bool operator<=(const market::proto::Container& lhs,
                const market::proto::Quantity& rhs);
bool operator>=(const market::proto::Container& lhs,
                const market::proto::Quantity& rhs);

} // namespace proto
} // namespace market

#endif
