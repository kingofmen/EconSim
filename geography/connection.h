// Class representing connections between areas.
#ifndef BASE_GEOGRAPHY_CONNECTION_H
#define BASE_GEOGRAPHY_CONNECTION_H

namespace geography {

class Connection {
 public:
   static std::unique_ptr<Connection> FromProto(const proto::Connection& conn);

 private:
  Connection() = delete;
  Connection(const proto::Connection& conn);

  // The underlying data.
  proto::Connection proto_;
};


} // namespace geography


#endif
