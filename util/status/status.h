// Utility methods for constructing Google Status objects.

#ifndef UTIL_STATUS_H
#define UTIL_STATUS_H

#include <string>

#include "absl/strings/str_format.h"
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

template <typename... Args>
Status AlreadyExistsErrorf(const absl::FormatSpec<Args...>& format,
                           const Args&... args) {
  return AlreadyExistsError(absl::StrFormat(format, args...));
}

template <typename... Args>
Status InvalidArgumentErrorf(const absl::FormatSpec<Args...>& format,
                             const Args&... args) {
  return InvalidArgumentError(absl::StrFormat(format, args...));
}
template <typename... Args>
Status FailedPreconditionErrorf(const absl::FormatSpec<Args...>& format,
                                const Args&... args) {
  return FailedPreconditionError(absl::StrFormat(format, args...));
}
template <typename... Args>
Status NotFoundErrorf(const absl::FormatSpec<Args...>& format,
                      const Args&... args) {
  return NotFoundError(absl::StrFormat(format, args...));
}
template <typename... Args>
Status NotImplementedErrorf(const absl::FormatSpec<Args...>& format,
                            const Args&... args) {
  return NotImplementedError(absl::StrFormat(format, args...));
}
template <typename... Args>
Status ResourceExhaustedErrorf(const absl::FormatSpec<Args...>& format,
                               const Args&... args) {
  return ResourceExhaustedError(absl::StrFormat(format, args...));
}

// Statuses are equal if they have the same code and message.
bool Equal(const Status& one, const Status& two);

// Special status indicating the function encountered no errors,
// but has not completed - something remains to be done.
const Status& NotComplete();

// Returns true if the candidate status is NotComplete.
bool IsNotComplete(const Status& cand);

} // namespace util

#endif // UTIL_STATUS_H
