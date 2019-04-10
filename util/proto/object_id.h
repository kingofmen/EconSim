#ifndef UTIL_PROTO_OBJECT_ID_H
#define UTIL_PROTO_OBJECT_ID_H

#include "util/proto/object_id.pb.h"
#include "util/headers/int_types.h"

// Hasher and equality operator for IDs, so they can be used as map keys.
namespace std {

template <> class hash<util::proto::ObjectId> {
public:
  size_t operator()(const util::proto::ObjectId& unit_id) const {
    static hash<uint64> hasher;
    return 31 * hasher(unit_id.type()) + hasher(unit_id.number());
  }
};

template <> struct equal_to<util::proto::ObjectId> {
  bool operator()(const util::proto::ObjectId& lhs,
                  const util::proto::ObjectId& rhs) const {
    return lhs.type() == rhs.type() && lhs.number() == rhs.number();
  }
};

}

#endif
