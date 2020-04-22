#include "games/setup/setup.h"

#include <experimental/filesystem>

#include "games/setup/validation/validation.h"
#include "games/factions/proto/factions.pb.h"
#include "games/geography/connection.h"
#include "games/geography/geography.h"
#include "games/industry/decisions/production_evaluator.h"
#include "games/market/goods_utils.h"
#include "games/market/market.h"
#include "games/units/unit.h"
#include "util/arithmetic/microunits.h"
#include "util/keywords/keywords.h"
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

  for (const auto& prod : proto.auto_production()) {
    auto_production_.push_back(prod);
  }
  for (const auto& chain : proto.production_chains()) {
    production_chains_.push_back(chain);
  }

  for (const auto& tag_decay_rate : proto.tag_decay_rates().quantities()) {
    market::SetAmount(tag_decay_rate.first,
                      micro::kOneInU - tag_decay_rate.second, &decay_rates_);
    Log::Debugf("%s tag survival rate: %d",
                market::GetAmount(decay_rates_, tag_decay_rate.first));
  }

  for (const auto& level : proto.consumption()) {
    consumption_.push_back(level);
    if (market::GetAmount(level.tags(), keywords::kSubsistenceTag) > 0) {
      subsistence_.push_back(level);
    }
  }

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
    auto& curr = world->areas_.back();
    if (!curr) {
      Log::Errorf("Could not load area: %s", area.DebugString());
      continue;
    }
    if (curr->Proto()->has_area_id()) {
      const auto& id = curr->Proto()->area_id();
      if (!id.has_kind()) {
        curr->Proto()->mutable_area_id()->set_kind("area");
      }
    }
  }

  for (const auto& conn : proto.connections()) {
    world->connections_.emplace_back(geography::Connection::FromProto(conn));
    auto& curr = world->connections_.back();
    if (!curr) {
      Log::Errorf("Could not load connection: %s", conn.DebugString());
      world->connections_.pop_back();
      continue;
    }
    if (!curr->Proto().a_area_id().has_kind()) {
      curr->mutable_proto()->mutable_a_area_id()->set_kind("area");
    }
    if (!curr->Proto().z_area_id().has_kind()) {
      curr->mutable_proto()->mutable_z_area_id()->set_kind("area");
    }
  }

  for (const auto& unit : proto.units()) {
    world->units_.emplace_back(units::Unit::FromProto(unit));
    if (!world->units_.back()) {
      Log::Warnf("Could not create unit from proto: %s", unit.DebugString());
      world->units_.pop_back();
    }
  }

  for (const auto& faction : proto.factions()) {
    world->factions_.emplace_back(factions::FactionController::FromProto(faction));
    auto& curr = world->factions_.back();
    if (!curr) {
      Log::Errorf("Could not load faction: %s", faction.DebugString());
      continue;
    }
    if (curr->Proto().has_faction_id()) {
      const auto& id = curr->Proto().faction_id();
      if (!id.has_kind() && !id.has_type()) {
        curr->mutable_proto()->mutable_faction_id()->set_kind("faction");
      }
    }
  }
  return world;
}

void tagIfPossible(util::proto::ObjectId* proto) {
  std::string tag = util::objectid::Tag(*proto);
  if (!tag.empty()) {
    proto->clear_number();
    proto->set_tag(tag);
  }
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

  for (auto& conn : connections_) {
    auto* proto = conn->mutable_proto();
    tagIfPossible(proto->mutable_a_area_id());
    tagIfPossible(proto->mutable_z_area_id());
  }

  for (auto& unit : units_) {
    auto* loc = unit->mutable_location();
    tagIfPossible(loc->mutable_a_area_id());
    if (loc->has_z_area_id()) {
      tagIfPossible(loc->mutable_z_area_id());
    }
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

util::Status
LoadExtras(const proto::ScenarioFiles& config,
           std::unordered_map<std::string, google::protobuf::Message*> extras) {
  std::experimental::filesystem::path base_path = config.root_path();
  const auto& locations = config.extras();
  for (auto& extra : extras) {
    const std::string& key = extra.first;
    if (locations.find(key) == locations.end()) {
      Log::Warnf("Could not load expected extra field \"%s\", it is not in the "
                 "location map.",
                 key);
      continue;
    }
    std::experimental::filesystem::path extra_path =
        base_path / locations.at(key);
    auto status =
        util::proto::MergeProtoFile(extra_path.string(), extra.second);
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
  for (auto& conn : *(world->mutable_connections())) {
    auto status = util::objectid::Canonicalise(conn.mutable_a_area_id());
    if (!status.ok()) {
      return status;
    }
    status = util::objectid::Canonicalise(conn.mutable_z_area_id());
    if (!status.ok()) {
      return status;
    }
  }

  for (auto& unit : *(world->mutable_units())) {
    if (unit.has_location()) {
      auto* loc = unit.mutable_location();
      if (loc->has_a_area_id()) {
        auto status = util::objectid::Canonicalise(loc->mutable_a_area_id());
        if (!status.ok()) {
          return status;
        }
      }
      if (loc->has_z_area_id()) {
        auto status = util::objectid::Canonicalise(loc->mutable_z_area_id());
        if (!status.ok()) {
          return status;
        }
      }
    }
    if (unit.has_strategy()) {
      if (unit.strategy().has_shuttle_trade()) {
        auto* st = unit.mutable_strategy()->mutable_shuttle_trade();
        if (st->has_area_a_id()) {
          auto status = util::objectid::Canonicalise(st->mutable_area_a_id());
          if (!status.ok()) {
            return status;
          }
        }
        if (st->has_area_z_id()) {
          auto status = util::objectid::Canonicalise(st->mutable_area_z_id());
          if (!status.ok()) {
            return status;
          }
        }
      } else if (unit.strategy().has_seven_years_merchant()) {
        auto* st = unit.mutable_strategy()->mutable_seven_years_merchant();
        if (st->has_base_area_id()) {
          auto status = util::objectid::Canonicalise(st->mutable_base_area_id());
          if (!status.ok()) {
            return status;
          }
        }
      }
    }
  }

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
