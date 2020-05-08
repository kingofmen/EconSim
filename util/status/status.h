// Utility methods for constructing Google Status objects.

#ifndef UTIL_STATUS_H
#define UTIL_STATUS_H

#include <string>

#include "src/google/protobuf/stubs/status.h"

namespace util {

typedef google::protobuf::util::Status Status;

const Status& OkStatus();
Status AlreadyExistsError(const std::string& msg);
Status InvalidArgumentError(const std::string& msg);
Status FailedPreconditionError(const std::string& msg);
Status NotFoundError(const std::string& msg);
Status NotImplementedError(const std::string& msg);
Status ResourceExhaustedError(const std::string& msg);

// Statuses are equal if they have the same code and message.
bool Equal(const Status& one, const Status& two);

} // namespace util

#endif // UTIL_STATUS_H
