// Utility functions for reading protobufs from files and writing them back.

#include <string>

#include "src/google/protobuf/message.h"
#include "util/status/status.h"


namespace util {
namespace proto {

// Parses filename into proto, returning a non-OK Status on failure. The proto
// (which cannot be null) is cleared before parsing, whether it succeeds or not.
util::Status ParseProtoFile(const std::string& filename,
                            google::protobuf::Message* proto);

// Parses filename into proto without clearing, in effect merging the file
// into the protobuf.
util::Status MergeProtoFile(const std::string& filename,
                            google::protobuf::Message* proto);

}  // namespace proto
}  // namespace util

