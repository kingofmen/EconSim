#include "util/proto/object_id.h"

#include "absl/strings/substitute.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace std {

template <> class hash<std::pair<uint64, uint64>> {
public:
  size_t operator()(const std::pair<uint64, uint64>& p) const {
    static hash<uint64> hasher;
    return 31 * hasher(p.first) + hasher(p.second);
  }
};


template <> class hash<std::pair<uint64, string>> {
public:
  size_t operator()(const std::pair<uint64, string>& p) const {
    static hash<uint64> hasher;
    static hash<string> str_hasher;
    return 31 * hasher(p.first) + str_hasher(p.second);
  }
};

}  // namespace std

std::unordered_map<std::pair<uint64, uint64>, std::string> numToTagMap;
std::unordered_map<std::pair<uint64, std::string>, uint64> tagToNumMap;

namespace util {
namespace objectid {

util::Status Canonicalise(util::proto::ObjectId* obj_id) {
  if (!obj_id->has_type()) {
    return util::InvalidArgumentError(absl::Substitute(
        "Cannot canonicalise object ID without type: $0",
        obj_id->DebugString()));
  }

  if (obj_id->has_number()) {
    if (!obj_id->has_tag()) {
      // Already canonical.
      return util::OkStatus();
    }

    // Original object.
    auto numkey = std::make_pair(obj_id->type(), obj_id->number());
    if (numToTagMap.find(numkey) != numToTagMap.end()) {
      return util::AlreadyExistsError(absl::Substitute(
          "Object $0 (number) already exists", obj_id->DebugString()));
    }

    auto tagkey = std::make_pair(obj_id->type(), obj_id->tag());
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
        "Object ID with only type: $0", obj_id->DebugString()));
  }

  auto tagkey = std::make_pair(obj_id->type(), obj_id->tag());
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
  auto numkey = std::make_pair(obj_id.type(), obj_id.number());
  return numToTagMap[numkey];
}

}  // namespace objectid
}  // namespace util
