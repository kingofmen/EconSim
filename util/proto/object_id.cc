#include "util/proto/object_id.h"

#include "absl/strings/substitute.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace std {

template <> class hash<std::pair<uint64, uint64>> {
public:
  size_t operator()(const pair<uint64, uint64>& p) const {
    static hash<uint64> hasher;
    return 31 * hasher(p.first) + hasher(p.second);
  }
};


template <> class hash<pair<string, uint64>> {
public:
  size_t operator()(const pair<string, uint64>& p) const {
    static hash<string> str_hasher;
    static hash<uint64> hasher;
    return 31 * str_hasher(p.first) + hasher(p.second);
  }
};

template <> class hash<pair<string, string>> {
public:
  size_t operator()(const pair<string, string>& p) const {
    static hash<string> hasher;
    return 31 * hasher(p.first) + hasher(p.second);
  }
};

}  // namespace std

std::unordered_map<std::pair<std::string, uint64>, std::string> numToTagMap;
std::unordered_map<std::pair<std::string, std::string>, uint64> tagToNumMap;

namespace util {
namespace objectid {

const util::proto::ObjectId kNullId;

util::Status Canonicalise(util::proto::ObjectId* obj_id) {
  if (!obj_id->has_type() && !obj_id->has_kind()) {
    return util::InvalidArgumentError(absl::Substitute(
        "Cannot canonicalise object ID without type or kind: $0",
        obj_id->DebugString()));
  }

  std::string kind = obj_id->kind();
  if (kind.empty()) {
    kind = absl::Substitute("$0", obj_id->type());
  }

  if (obj_id->has_number()) {
    if (!obj_id->has_tag()) {
      // Already canonical.
      return util::OkStatus();
    }

    // Original object.
    auto numkey = std::make_pair(kind, obj_id->number());
    if (numToTagMap.find(numkey) != numToTagMap.end()) {
      return util::AlreadyExistsError(absl::Substitute(
          "Object $0 (number) already exists", obj_id->DebugString()));
    }

    auto tagkey = std::make_pair(kind, obj_id->tag());
    if (tagToNumMap.find(tagkey) != tagToNumMap.end()) {
      return util::AlreadyExistsError(absl::Substitute(
          "Object $0 (tag) already exists", obj_id->DebugString()));
    }

    numToTagMap[numkey] = obj_id->tag();
    tagToNumMap[tagkey] = obj_id->number();
    obj_id->clear_tag();
    return util::OkStatus();
  }

  if (!obj_id->has_tag()) {
    return util::InvalidArgumentError(absl::Substitute(
        "Object ID with no number or tag: $0", obj_id->DebugString()));
  }

  auto tagkey = std::make_pair(kind, obj_id->tag());
  if (tagToNumMap.find(tagkey) == tagToNumMap.end()) {
    return util::NotFoundError(absl::Substitute(
        "Cannot find number for tagged ID: $0", obj_id->DebugString()));
  }
  obj_id->set_number(tagToNumMap[tagkey]);
  obj_id->clear_tag();
  return util::OkStatus();
}

std::string Tag(const util::proto::ObjectId& obj_id) {
  if (obj_id.has_tag()) {
    return obj_id.tag();
  }
  std::string kind = obj_id.kind();
  if (kind.empty()) {
    kind = absl::Substitute("$0", obj_id.type());
  }
  auto numkey = std::make_pair(kind, obj_id.number());
  return numToTagMap[numkey];
}

bool Equal(const util::proto::ObjectId& one, const util::proto::ObjectId& two) {
  static std::equal_to<util::proto::ObjectId> ids_equal;
  return ids_equal(one, two);
}

bool IsNull(const util::proto::ObjectId& obj_id) {
  return Equal(obj_id, kNullId);
}

}  // namespace objectid
}  // namespace util

bool operator==(const util::proto::ObjectId& one,
                const util::proto::ObjectId& two) {
  return util::objectid::Equal(one, two);
}

bool operator!=(const util::proto::ObjectId& one,
                const util::proto::ObjectId& two) {
  return !(one == two);
}


