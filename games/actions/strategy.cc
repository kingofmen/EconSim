#include "games/actions/strategy.h"

#include <unordered_map>

#include "absl/strings/substitute.h"
#include "games/actions/proto/strategy.pb.h"
#include "util/status/status.h"

std::unordered_map<std::string, actions::proto::Strategy> registry;

namespace actions {

util::Status RegisterStrategy(const actions::proto::Strategy& strategy) {
  if (strategy.key_case() != actions::proto::Strategy::kDefine) {
    return util::InvalidArgumentError(
        "Cannot register a strategy without 'define' field set");
  }
  const std::string& name = strategy.define();
  if (registry.find(name) != registry.end()) {
    return util::AlreadyExistsError(
        absl::Substitute("Strategy $0 already registered", name));
  }
  registry[name] = strategy;
  return util::OkStatus();
}

util::Status LoadStrategy(const std::string& name,
                          actions::proto::Strategy* strategy) {
  if (strategy == nullptr) {
    return util::FailedPreconditionError(
        "Cannot copy into null Strategy pointer");
  }

  if (registry.find(name) == registry.end()) {
    return util::NotFoundError(
        absl::Substitute("Strategy $0 not found in registry", name));
  }

  *strategy = registry[name];
  strategy->clear_define();
  strategy->set_lookup(registry[name].define());
  return util::OkStatus();
}

const std::string& StepName(const actions::proto::Step& step) {
  static const std::string kUnknown("unknown step");
  switch (step.trigger_case()) {
    case actions::proto::Step::kKey:
      return step.key();
    case actions::proto::Step::kAction:
      return actions::proto::AtomicAction_Name(step.action());
    case actions::proto::Step::TRIGGER_NOT_SET:
    default:
      return kUnknown;
  }
  return kUnknown;
}


}  // namespace actions
