#ifndef UTIL_LOGGING_H
#define UTIL_LOGGING_H

#include <functional>
#include <string>

#include "absl/strings/str_format.h"

namespace Log {

enum Priority {
  P_UNKNOWN = 0,
  P_TRACE,
  P_DEBUG,
  P_INFO,
  P_WARN,
  P_ERROR,
  P_USER,
};

typedef std::function<void(const std::string& message, Priority)> callback;

void Register(callback listener, Priority minimum = P_TRACE);
void UnRegister(callback listener);
void Trace(const std::string& message);
void Debug(const std::string& message);
void Info(const std::string& message);
void Warn(const std::string& message);
void Error(const std::string& message);
void User(const std::string& message);
void Stream(Priority p, const std::string& message);

template <typename... Args>
void Tracef(const absl::FormatSpec<Args...>& format, const Args&... args) {
  Trace(absl::StrFormat(format, args...));
}
template <typename... Args>
void Debugf(const absl::FormatSpec<Args...>& format, const Args&... args) {
  Debug(absl::StrFormat(format, args...));
}
template <typename... Args>
void Infof(const absl::FormatSpec<Args...>& format, const Args&... args) {
  Info(absl::StrFormat(format, args...));
}
template <typename... Args>
void Warnf(const absl::FormatSpec<Args...>& format, const Args&... args) {
  Warn(absl::StrFormat(format, args...));
}
template <typename... Args>
void Errorf(const absl::FormatSpec<Args...>& format, const Args&... args) {
  Error(absl::StrFormat(format, args...));
}
template <typename... Args>
void Userf(const absl::FormatSpec<Args...>& format, const Args&... args) {
  User(absl::StrFormat(format, args...));
}
template <typename... Args>
void Streamf(Priority p, const absl::FormatSpec<Args...>& format, const Args&... args) {
  switch (p) {
    case P_TRACE:
      Tracef(format, args...);
      break;
    case P_DEBUG:
      Debugf(format, args...);
      break;
    case P_INFO:
      Infof(format, args...);
      break;
    case P_WARN:
      Warnf(format, args...);
      break;
    case P_ERROR:
      Errorf(format, args...);
      break;
    case P_USER:
      Userf(format, args...);
      break;
    default:
      break;
  }
}

} // namespace Log

#endif
