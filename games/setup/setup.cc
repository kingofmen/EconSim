#include "games/setup/setup.h"

#include <experimental/filesystem>

#include "games/setup/validation/validation.h"
#include "factions/proto/factions.pb.h"
#include "geography/connection.h"
#include "geography/geography.h"
#include "industry/decisions/production_evaluator.h"
#include "market/market.h"
#include "units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/proto/file.h"
#include "util/status/status.h"
#include "util/logging/logging.h"

namespace games {
namespace setup {

Constants::Constants(const games::setup::proto::Scenario& proto) {
  for (const auto& good : proto.trade_goods()) {
    auto& name = good.name();
    market::CreateTradeGood(good);
    market::SetAmount(name, micro::kOneInU - good.decay_rate_u(),
                      &decay_rates_);
    Log::Debugf("Created %s with survival rate %d", good.name(),
                market::GetAmount(decay_rates_, name));
  }
  auto_production_.insert(auto_production_.end(),
                          proto.auto_production().pointer_begin(),
                          proto.auto_production().pointer_end());
  production_chains_.insert(production_chains_.end(),
                            proto.production_chains().pointer_begin(),
                            proto.production_chains().pointer_end());

  for (const auto& temp : proto.unit_templates()) {
    units::Unit::RegisterTemplate(temp);
  }
}

std::unique_ptr<World> World::FromProto(const proto::GameWorld& proto){
  auto world = std::make_unique<World>();
  for (const auto& pop : proto.pops()) {
    world->pops_.emplace_back(new population::PopUnit(pop));
  }

  for (const auto& area : proto.areas()) {
    world->areas_.emplace_back(geography::Area::FromProto(area));
  }

  for (const auto& conn : proto.connections()) {
    world->connections_.emplace_back(geography::Connection::FromProto(conn));
  }

  for (const auto& unit : proto.units()) {
    world->units_.emplace_back(units::Unit::FromProto(unit));
  }

  for (const auto& faction : proto.factions()) {
    world->factions_.emplace_back(factions::FactionController::FromProto(faction));
  }
  return world;
}

void World::restoreTags() {
  for (auto& area : areas_) {
    auto* area_proto = area->Proto();
    if (!area_proto->has_area_id()) {
      continue;
    }
    std::string tag = util::objectid::Tag(area_proto->area_id());
    if (tag.empty()) {
      continue;
    }
    area_proto->mutable_area_id()->set_tag(tag);
  }
  for (auto& faction : factions_) {
    auto* faction_proto = faction->mutable_proto();
    if (!faction_proto->has_faction_id()) {
      continue;
    }
    std::string tag = util::objectid::Tag(faction_proto->faction_id());
    if (tag.empty()) {
      continue;
    }
    faction_proto->mutable_faction_id()->set_tag(tag);
  }
}

util::Status World::ToProto(proto::GameWorld* proto) {
  restoreTags();
  for (const auto& pop : pops_) {
    *proto->add_pops() = *pop->Proto();
  }
  for (const auto& area : areas_) {
    auto* area_proto = proto->add_areas();
    *area_proto = *area->Proto();
    if (area_proto->has_market()) {
      auto* market = area->mutable_market();
      auto* market_proto = area_proto->mutable_market();
      *market_proto = *market->Proto();
    }
  }
  for (const auto& conn : connections_) {
    auto* conn_proto = proto->add_connections();
    *conn_proto = conn->Proto();
  }

  for (const auto& unit : units_) {
    market::CleanContainer(unit->mutable_resources());
    auto& unit_proto = *proto->add_units();
    unit_proto = unit->Proto();
  }

  for (const auto& faction : factions_) {
    auto* faction_proto = proto->add_factions();
    *faction_proto = faction->Proto();
  }

  return util::OkStatus();
}


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

util::Status CanonicaliseScenario(proto::Scenario* scenario) {
  return util::OkStatus();
}

util::Status CanonicaliseWorld(proto::GameWorld* world) {
  // First canonicalise all self-id objects.
  for (auto& faction : *(world->mutable_factions())) {
    auto status = util::objectid::Canonicalise(faction.mutable_faction_id());
    if (!status.ok()) {
      return status;
    }
  }
  for (auto& area : *(world->mutable_areas())) {
    auto status = util::objectid::Canonicalise(area.mutable_area_id());
    if (!status.ok()) {
      return status;
    }
  }

  // Only now do the references.
  
  return util::OkStatus();
}

util::Status CreateWorld(const proto::ScenarioFiles& config,
                         std::unique_ptr<World>& world, Constants* constants) {
  if (world) {
    return util::InvalidArgumentError(
        "Non-empty world pointer passed to CreateWorld.");
  }

  proto::GameWorld world_proto;
  proto::Scenario scenario_proto;
  auto status = LoadScenario(config, &scenario_proto);
  if (!status.ok()) {
    return status;
  }
  std::vector<std::string> errors =
      validation::Validate(scenario_proto, world_proto);
  if (!errors.empty()) {
    for (const auto err : errors) {
      Log::Error(err);
    }
    return util::InvalidArgumentError("Validation errors in scenario");
  }

  *constants = Constants(scenario_proto);

  status = LoadWorld(config, &world_proto);
  if (!status.ok()) {
    return status;
  }
  world = World::FromProto(world_proto);
  return util::OkStatus();
}

}  // namespace setup
}  // namespace games
