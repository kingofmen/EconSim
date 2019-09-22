#include "util/logging/logging.h"

#include <algorithm>
#include <string>
#include <vector>

namespace Log{
namespace internal {

struct listener {
  listener(callback h, Priority m) : hear(h), minimum(m) {}
  callback hear;
  Priority minimum;
};

std::vector<listener> listeners;

void Log(const std::string& message, Priority p) {
  for (auto& l : listeners) {
    if (l.minimum > p) {
      continue;
    }
    l.hear(message, p);
  }
}

}  // namespace internal

void UnRegister(callback c) {
  auto newend =
      std::remove_if(internal::listeners.begin(), internal::listeners.end(),
                     [&](const internal::listener& l) {
                       return l.hear.target<callback>() == c.target<callback>();
                     });
  internal::listeners.erase(newend, internal::listeners.end());
}

void Register(callback l, Priority m) {
  if (l == NULL) {
    return;
  }
  internal::listeners.emplace_back(l, m);
}

void Trace(const std::string& message) {
  internal::Log(message, P_TRACE);
}
void Debug(const std::string& message) {
  internal::Log(message, P_DEBUG);
}
void Info(const std::string& message) {
  internal::Log(message, P_INFO);
}
void Warn(const std::string& message) {
  internal::Log(message, P_WARN);
}
void Error(const std::string& message) {
  internal::Log(message, P_ERROR);
}
void User(const std::string& message) {
  internal::Log(message, P_USER);
}
void Stream(const std::string& message, Priority p) {
  switch (p) {
    case P_TRACE:
      Trace(message);
      break;
    case P_DEBUG:
      Debug(message);
      break;
    case P_INFO:
      Info(message);
      break;
    case P_WARN:
      Warn(message);
      break;
    case P_ERROR:
      Error(message);
      break;
    case P_USER:
      User(message);
      break;
    default:
      break;
  }
}

}  // namespace Log
