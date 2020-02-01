#include <experimental/filesystem>
#include <vector>

#include "absl/strings/substitute.h"
#include "game/setup/proto/setup.pb.h"
#include "util/logging/logging.h"
#include "util/status/status.h"

google::protobuf::util::Status
validateSetup(const game::setup::proto::ScenarioFiles& setup) {
  if (!setup.has_name()) {
    return util::InvalidArgumentError(
        absl::Substitute("Setup file has no name"));
  }
  if (!setup.has_description()) {
    return util::InvalidArgumentError(
        absl::Substitute("$0 has no description", setup.name()));
  }
  return util::OkStatus();
}

std::vector<std::experimental::filesystem::path> getScenarios() {
  auto current_path = std::experimental::filesystem::current_path();
  current_path /= "scenarios";

  if (!std::experimental::filesystem::exists(current_path)) {
    return {};
  }
  auto file_it = std::experimental::filesystem::directory_iterator(current_path);
  auto end = std::experimental::filesystem::directory_iterator();
  std::vector<std::experimental::filesystem::path> scenarios;
  for (; file_it != end; ++file_it) {
    if (file_it->path().extension() != ".scenario") {
      continue;
    }
    scenarios.push_back(file_it->path());
  }

  return scenarios;
}

int main(int /*argc*/, char** /*argv*/) {
  Log::Register(Log::coutLogger);
  auto scenarios = getScenarios();
  if (scenarios.empty()) {
    Log::Error("No scenarios found");
    return 1;
  }
  return 0;
}
