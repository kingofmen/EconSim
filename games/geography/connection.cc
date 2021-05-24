#include "games/geography/connection.h"

#include <functional>

#include "util/proto/object_id.h"

std::unordered_map<util::proto::ObjectId,
                   std::unordered_set<geography::Connection*>>
    endpoint_map_;
std::unordered_map<uint64, std::unordered_set<geography::Connection*>>
    both_endpoints_map_;
std::unordered_map<geography::Connection::IdType, geography::Connection*> id_map_;
std::equal_to<util::proto::ObjectId> ids_equal;

namespace geography {
namespace {

uint64 Fingerprint(const util::proto::ObjectId& one,
                   const util::proto::ObjectId& two) {
  static std::hash<util::proto::ObjectId> hasher;
  return 31 * hasher(one) + hasher(two);
}

} // namespace

Connection::Connection(const proto::Connection& conn) : proto_(conn) {
  endpoint_map_[proto_.a_area_id()].insert(this);
  endpoint_map_[proto_.z_area_id()].insert(this);
  both_endpoints_map_[Fingerprint(proto_.a_area_id(), proto_.z_area_id())]
      .insert(this);
  both_endpoints_map_[Fingerprint(proto_.z_area_id(), proto_.a_area_id())]
      .insert(this);
  id_map_[proto_.connection_id()] = this;
}

Connection::~Connection() {
  endpoint_map_[proto_.a_area_id()].erase(this);
  endpoint_map_[proto_.z_area_id()].erase(this);
  both_endpoints_map_[Fingerprint(proto_.a_area_id(), proto_.z_area_id())]
      .erase(this);
  both_endpoints_map_[Fingerprint(proto_.z_area_id(), proto_.a_area_id())]
      .erase(this);
  id_map_.erase(connection_id());
}

void Connection::Register(const util::proto::ObjectId& unit_id,
                          Connection::Listener l) {
  listeners_[unit_id] = l;
}

void Connection::UnRegister(const util::proto::ObjectId& unit_id) {
  listeners_.erase(unit_id);
}

std::unique_ptr<Connection>
Connection::FromProto(const proto::Connection& conn) {
  std::unique_ptr<Connection> ret;
  // TODO: Actually handle these errors.
  if (util::objectid::IsNull(conn.connection_id())) {
    return ret;
  }
  if (ById(conn.connection_id()) != NULL) {
    return ret;
  }
  if (!conn.has_a_area_id()) {
    return ret;
  }
  if (!conn.has_z_area_id()) {
    return ret;
  }
  if (ids_equal(conn.a_area_id(), conn.z_area_id())) {
    return ret;
  }
  if (conn.distance_u() == 0) {
    return ret;
  }
  if (conn.width_u() == 0) {
    return ret;
  }

  ret.reset(new Connection(conn));
  return ret;
}

const std::unordered_set<Connection*>&
Connection::ByEndpoint(const util::proto::ObjectId& area_id) {
  return endpoint_map_[area_id];
}

const std::unordered_set<Connection*>&
Connection::ByEndpoints(const util::proto::ObjectId& area_one,
                        const util::proto::ObjectId& area_two) {
  return both_endpoints_map_[Fingerprint(area_one, area_two)];
}

Connection* Connection::ById(const Connection::IdType& conn_id) {
  if (id_map_.find(conn_id) == id_map_.end()) {
    return nullptr;
  }
  return id_map_.at(conn_id);
}

const util::proto::ObjectId&
Connection::OtherSide(const util::proto::ObjectId& area_id) const {
  std::equal_to<util::proto::ObjectId> comp;
  if (ids_equal(area_id, proto_.a_area_id())) {
    return proto_.z_area_id();
  }
  if (ids_equal(area_id, proto_.z_area_id())) {
    return proto_.a_area_id();
  }
  return util::objectid::kNullId;
}

Area* Connection::OtherSide(const Area* area) {
  if (area == a()) {
    return z();
  }
  if (area == z()) {
    return a();
  }
  return NULL;
}

const Area* Connection::OtherSide(const Area* area) const {
  if (area == a()) {
    return z();
  }
  if (area == z()) {
    return a();
  }
  return NULL;
}

void Connection::Listen(const Mobile& mobile, uint64 distance_u,
                        std::vector<Connection::Detection>* detections) const {
  for (const auto& listener : listeners_) {
    Connection::Detection det = listener.second(mobile);
    det.unit_id = listener.first;
    detections->emplace_back(std::move(det));
  }
}

} // namespace geography
