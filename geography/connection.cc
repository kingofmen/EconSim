#include "geography/connection.h"

namespace geography {

std::unordered_map<uint64, std::unordered_set<Connection*>>
    Connection::endpoint_map_;

Connection::Connection(const proto::Connection& conn) : proto_(conn) {
  endpoint_map_[proto_.a()].insert(this);
  endpoint_map_[proto_.z()].insert(this);
}

std::unique_ptr<Connection>
Connection::FromProto(const proto::Connection& conn) {
  std::unique_ptr<Connection> ret;
  // TODO: Actually handle these errors.
  if (conn.a() == 0) {
    return ret;
  }
  if (conn.z() == 0) {
    return ret;
  }
  if (conn.a() == conn.z()) {
    return ret;
  }
  if (conn.distance() == 0) {
    return ret;
  }
  if (conn.width() == 0) {
    return ret;
  }

  ret.reset(new Connection(conn));
  return ret;
}

const std::unordered_set<Connection*>& Connection::ByEndpoint(uint64 area_id) {
  return endpoint_map_[area_id];
}

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

} // namespace geography
