#ifndef UTIL_PROTO_OBJECT_ID_H
#define UTIL_PROTO_OBJECT_ID_H

#include <string>

#include "util/proto/object_id.pb.h"
#include "util/headers/int_types.h"
#include "util/status/status.h"

namespace util {
namespace objectid {

// Sets the object ID to its canonical type-number form if it has a tag.
// If it has both number and tag, stores the tag-number mapping and removes
// the tag.
util::Status Canonicalise(util::proto::ObjectId* obj_id);

// Returns the associated tag.
std::string Tag(const util::proto::ObjectId& obj_id);

}  // namespace objectid
}  // namespace util

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
