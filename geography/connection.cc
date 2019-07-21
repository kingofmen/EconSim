#include "geography/connection.h"

#include <functional>

std::unordered_map<uint64, std::unordered_set<geography::Connection*>>
    geography::Connection::endpoint_map_;
std::unordered_map<uint64, std::unordered_set<geography::Connection*>>
    geography::Connection::both_endpoints_map_;
std::unordered_map<uint64, geography::Connection*>
    geography::Connection::id_map_;

namespace geography {
namespace {

uint64 Fingerprint(uint64 one, uint64 two) {
  static std::hash<uint64> hasher;
  return 31 * hasher(one) + hasher(two);
}

} // namespace

Connection::Connection(const proto::Connection& conn) : proto_(conn) {
  endpoint_map_[proto_.a()].insert(this);
  endpoint_map_[proto_.z()].insert(this);
  both_endpoints_map_[Fingerprint(proto_.a(), proto_.z())].insert(this);
  both_endpoints_map_[Fingerprint(proto_.z(), proto_.a())].insert(this);
  id_map_[proto_.id()] = this;
}

Connection::~Connection() {
  endpoint_map_[proto_.a()].erase(this);
  endpoint_map_[proto_.z()].erase(this);
  both_endpoints_map_[Fingerprint(proto_.a(), proto_.z())].erase(this);
  both_endpoints_map_[Fingerprint(proto_.z(), proto_.a())].erase(this);
  id_map_.erase(ID());
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
  if (conn.id() == 0) {
    return ret;
  }
  if (ById(conn.id()) != NULL) {
    return ret;
  }
  if (conn.a() == 0) {
    return ret;
  }
  if (conn.z() == 0) {
    return ret;
  }
  if (conn.a() == conn.z()) {
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

const std::unordered_set<Connection*>& Connection::ByEndpoint(uint64 area_id) {
  return endpoint_map_[area_id];
}

const std::unordered_set<Connection*>&
Connection::ByEndpoints(uint64 area_one, uint64 area_two) {
  return both_endpoints_map_[Fingerprint(area_one, area_two)];
}

Connection* Connection::ById(uint64 conn_id) { return id_map_[conn_id]; }

uint64 Connection::OtherSide(uint64 area_id) const {
  if (area_id == proto_.a()) {
    return proto_.z();
  }
  if (area_id == proto_.z()) {
    return proto_.a();
  }
  return 0;
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

bool DefaultTraverser::Traverse(const Mobile& mobile,
                                proto::Location* location) const {
  if (!location->has_connection_id()) {
    return false;
  }
  Connection* conn = Connection::ById(location->connection_id());
  if (conn == NULL) {
    // TODO: Handle this error case.
    return false;
  }

  uint64 progress_u = location->progress_u();
  uint64 distance_u = mobile.speed_u(conn->type());
  uint64 length_u = conn->length_u() - progress_u;
  if (distance_u > length_u) {
    distance_u = length_u;
  }

  std::vector<Connection::Detection> detections;
  conn->Listen(mobile, distance_u, &detections);
  // TODO: Actually handle detections.

  if (distance_u >= length_u) {
    location->clear_progress_u();
    location->set_source_area_id(conn->OtherSide(location->source_area_id()));
    location->clear_connection_id();
    if (location->source_area_id() == location->destination_area_id()) {
      location->clear_destination_area_id();
      return true;
    }
  } else {
    location->set_progress_u(progress_u + distance_u);
  }

  return false;
}

} // namespace geography
