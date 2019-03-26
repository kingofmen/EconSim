// Utility functions for reading protobufs from files and writing them back.

#include <string>

#include "src/google/protobuf/message.h"
#include "util/status/status.h"

namespace util {
namespace proto {

// Parses filename into proto, returning a non-OK Status on failure.
util::Status ParseProtoFile(const std::string& filename,
                            google::protobuf::Message* proto);

}  // namespace proto
}  // namespace util

