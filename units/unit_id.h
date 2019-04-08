#ifndef UNITS_UNIT_ID_H
#define UNITS_UNIT_ID_H

#include "units/proto/units.pb.h"
#include "util/headers/int_types.h"

// Hasher and equality operator for unit IDs, so they can be used as map keys.
namespace std {

template <> class hash<units::proto::UnitId> {
public:
  size_t operator()(const units::proto::UnitId& unit_id) const {
    static hash<uint64> hasher;
    return 31 * hasher(unit_id.type()) + hasher(unit_id.number());
  }
};

template <> struct equal_to<units::proto::UnitId> {
  bool operator()(const units::proto::UnitId& lhs,
                  const units::proto::UnitId& rhs) const {
    return lhs.type() == rhs.type() && lhs.number() == rhs.number();
  }
};

}

#endif
