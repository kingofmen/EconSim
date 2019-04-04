// Utility methods for constructing Google Status objects.

#ifndef UTIL_STATUS_H
#define UTIL_STATUS_H

#include <string>

#include "src/google/protobuf/stubs/status.h"

namespace util {

const google::protobuf::util::Status& OkStatus();
google::protobuf::util::Status InvalidArgumentError(const std::string& msg);
google::protobuf::util::Status FailedPreconditionError(const std::string& msg);

} // namespace util

#endif // UTIL_STATUS_H
