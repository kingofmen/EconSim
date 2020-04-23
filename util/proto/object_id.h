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

// Returns true if the ObjectIds are equal.
bool Equal(const util::proto::ObjectId& one, const util::proto::ObjectId& two);

// Returns true if obj_id compares equal to a null ObjectId with no fields set.
bool IsNull(const util::proto::ObjectId& obj_id);

// Returns a string suitable for display to humans.
std::string DisplayString(const util::proto::ObjectId& obj_id);

extern const util::proto::ObjectId kNullId;

}  // namespace objectid
}  // namespace util

// Hasher and equality operator for IDs, so they can be used as map keys.
namespace std {

template <> class hash<util::proto::ObjectId> {
public:
  size_t operator()(const util::proto::ObjectId& object_id) const {
    static hash<uint64> hasher;
    static hash<std::string> str_hasher;
    if (object_id.has_kind()) {
      return 31 * str_hasher(object_id.kind()) + hasher(object_id.number());
    }
    return 31 * hasher(object_id.type()) + hasher(object_id.number());
  }
};

template <> struct equal_to<util::proto::ObjectId> {
  bool operator()(const util::proto::ObjectId& lhs,
                  const util::proto::ObjectId& rhs) const {
    if (lhs.number() != rhs.number()) {
      return false;
    }
    if (lhs.has_kind()) {
      return lhs.kind() == rhs.kind();
    }
    return lhs.type() == rhs.type();
  }
};

}

bool operator==(const util::proto::ObjectId& one,
                const util::proto::ObjectId& two);

bool operator!=(const util::proto::ObjectId& one,
                const util::proto::ObjectId& two);

#endif
