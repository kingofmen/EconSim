// Utility functions for manipulating goods.

#include "proto/goods.pb.h"

namespace market {

// Returns true if con has an entry for the good name, even if the amount is
// zero.
bool Contains(const market::proto::Container &con, const std::string name);
// Returns true if con has an entry for the good in qua. The amount is ignored.
bool Contains(const market::proto::Container &con,
              const market::proto::Quantity &qua);

// Returns the amount of name in con; zero if not set.
double GetAmount(const market::proto::Container &con, const std::string name);
// Returns the amount of qua.kind() in con; zero if not set.
double GetAmount(const market::proto::Container &con,
                 const market::proto::Quantity &qua);

// Sets all amounts in the container to 0.
void Clear(market::proto::Container &con);

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

// Dot product - for example, multiply a basket of goods by their prices.
double operator*(const market::proto::Container &lhs,
                 const market::proto::Container &rhs);

// Note that these are not stable - it is possible for (a < b) and (b < a) to be
// both true.
bool operator<(const market::proto::Container &lhs,
               const market::proto::Container &rhs);
bool operator>(const market::proto::Container &lhs,
               const market::proto::Container &rhs);
bool operator<=(const market::proto::Container &lhs,
                const market::proto::Container &rhs);
bool operator>=(const market::proto::Container &lhs,
                const market::proto::Container &rhs);
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
