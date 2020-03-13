#include "games/setup/setup.h"

#include <experimental/filesystem>
#include "util/proto/file.h"
#include "util/status/status.h"

namespace games {
namespace setup {

util::Status LoadScenario(const proto::ScenarioFiles& config,
                          proto::Scenario* scenario) {
  std::experimental::filesystem::path base_path = config.root_path();
  for (const auto& filename : config.auto_production()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), scenario);
    if (!status.ok()) {
      return status;
    }
  }
  for (const auto& filename : config.production_chains()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), scenario);
    if (!status.ok()) {
      return status;
    }
  }
  for (const auto& filename : config.trade_goods()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), scenario);
    if (!status.ok()) {
      return status;
    }
  }
  for (const auto& filename : config.consumption()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), scenario);
    if (!status.ok()) {
      return status;
    }
  }
  for (const auto& filename : config.unit_templates()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), scenario);
    if (!status.ok()) {
      return status;
    }
  }
  return util::OkStatus();
}

util::Status LoadWorld(const proto::ScenarioFiles& config,
                       proto::GameWorld* world) {
  std::experimental::filesystem::path base_path = config.root_path();
    std::experimental::filesystem::path world_path = base_path / config.world_file();
  auto status = util::proto::ParseProtoFile(world_path.string(), world);
  if (!status.ok()) {
    return status;
  }
  for (const auto& filename : config.factions()) {
    std::experimental::filesystem::path full_path = base_path / filename;
    auto status = util::proto::MergeProtoFile(full_path.string(), world);
    if (!status.ok()) {
      return status;
    }
  }

  return util::OkStatus();
}

}  // namespace setup
}  // namespace games
