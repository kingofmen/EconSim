#include "games/setup/validation/validation.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "absl/strings/substitute.h"
#include "games/market/proto/goods.pb.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.h"

std::unordered_map<std::string, market::proto::TradeGood> goods;
std::unordered_map<util::proto::ObjectId, geography::proto::Area> areas;
std::unordered_map<uint64, units::proto::Template> templates;
std::unordered_map<std::string, units::proto::Template> template_map;
std::unordered_map<uint64, geography::proto::Connection> connections;
std::unordered_map<uint64, population::proto::PopUnit> pops;
std::unordered_map<util::proto::ObjectId, units::proto::Unit> unit_map;
std::unordered_map<std::string, games::setup::validation::Validator>
    validator_map;

namespace games {
namespace setup {
namespace validation {
namespace {

// Validates that goods have sensible values.
void checkGoods(const games::setup::proto::Scenario& scenario,
                std::vector<std::string>* errors) {
  for (const market::proto::TradeGood& good : scenario.trade_goods()) {
    goods.emplace(good.name(), good);
    if (good.transport_type() != market::proto::TradeGood::TTT_IMMOBILE) {
      if (!good.has_bulk_u()) {
        errors->push_back(absl::Substitute("$0 has no bulk", good.name()));
      }
      if (0 >= good.weight_u()) {
        errors->push_back(absl::Substitute("$0 has no weight", good.name()));
      }
    }
    if (0 > good.decay_rate_u()) {
      errors->push_back(absl::Substitute("$0 has bad decay rate $1",
                                         good.name(), good.decay_rate_u()));
    }
  }
}

// Checks that all goods in the container exist.
void checkGoodsExist(const market::proto::Container& con,
                     const std::string& prefix,
                     std::vector<std::string>* errors) {
  for (const auto& q : con.quantities()) {
    const std::string& name = q.first;
    if (goods.find(name) == goods.end()) {
      errors->push_back(
          absl::Substitute("$0: Good $1 does not exist.", prefix, name));
    }
  }
}

// Checks that all autoproduction output exists.
void checkAutoProduction(const games::setup::proto::Scenario& scenario,
                         std::vector<std::string>* errors) {
  for (const population::proto::AutoProduction& ap :
       scenario.auto_production()) {
    checkGoodsExist(ap.output(), "Auto production", errors);
  }
}

// Checks that all production inputs, outputs, and capital exist.
void checkProduction(const games::setup::proto::Scenario& scenario,
                     std::vector<std::string>* errors) {
  for (const auto& prod : scenario.production_chains()) {
    std::string name = prod.name();
    if (name == "") {
      errors->push_back(
          absl::Substitute("Chain without name: $0", prod.DebugString()));
      name = "unnamed chain";
    }
    checkGoodsExist(prod.outputs(),
                    absl::Substitute("Production $0 outputs", name), errors);
    int step_count = 0;
    for (const auto& step : prod.steps()) {
      step_count++;
      int var_count = 0;
      for (const auto& var : step.variants()) {
        var_count++;
        checkGoodsExist(
            var.consumables(),
            absl::Substitute("Production $0 step $1 variant $2 consumables",
                             name, step_count, var_count),
            errors);
        checkGoodsExist(
            var.movable_capital(),
            absl::Substitute("Production $0 step $1 variant $2 movable capital",
                             name, step_count, var_count),
            errors);
        checkGoodsExist(
            var.fixed_capital(),
            absl::Substitute("Production $0 step $1 variant $2 fixed capital",
                             name, step_count, var_count),
            errors);
        checkGoodsExist(
            var.raw_materials(),
            absl::Substitute("Production $0 step $1 variant $2 raw materials",
                             name, step_count, var_count),
            errors);
        checkGoodsExist(
            var.install_cost(),
            absl::Substitute("Production $0 step $1 variant $2 install cost",
                             name, step_count, var_count),
            errors);
      }
    }
  }
}

// Checks that all consumed goods actually exist.
void checkConsumption(const games::setup::proto::Scenario& scenario,
                      std::vector<std::string>* errors) {
  for (const auto& con : scenario.consumption()) {
    std::string name = con.name();
    if (name == "") {
      errors->push_back(
          absl::Substitute("Consumption without name: $0", con.DebugString()));
      name = "unnamed level";
    }
    int package_count = 0;
    for (const auto& package : con.packages()) {
      package_count++;
      checkGoodsExist(package.consumed(),
                      absl::Substitute("Consumption $0 package $1 consumption",
                                       name, package_count),
                      errors);
      checkGoodsExist(package.capital(),
                      absl::Substitute("Consumption $0 package $1 capital",
                                       name, package_count),
                      errors);
    }
  }
}

// Sanity-checks templates.
void checkTemplates(const games::setup::proto::Scenario& scenario,
                    std::vector<std::string>* errors) {
  for (const auto& temp : scenario.unit_templates()) {
    if (!temp.has_template_id()) {
      errors->push_back(
          absl::Substitute("Template without ID: $0", temp.DebugString()));
      continue;
    }
    if (template_map.find(temp.template_id().kind()) != template_map.end()) {
      errors->push_back(absl::Substitute("Template kind is not unique: $0",
                                         temp.DebugString()));
      continue;
    }
    template_map[temp.template_id().kind()] = temp;
  }
}

// Checks that areas have valid IDs.
void validateAreas(const games::setup::proto::GameWorld& world,
                   std::vector<std::string>* errors) {
  for (const auto& area : world.areas()) {
    if (!area.has_area_id()) {
      errors->push_back(
          absl::Substitute("Area without ID: \"$0\"", area.DebugString()));
    } else if (!area.area_id().has_number() || area.area_id().number() < 1) {
      errors->push_back(
          absl::Substitute("Bad area ID: $0", area.area_id().DebugString()));
    } else if (areas.find(area.area_id()) != areas.end()) {
      errors->push_back(absl::Substitute("Area ID $0 is not unique",
                                         area.area_id().DebugString()));
    } else {
      areas[area.area_id()] = area;
    }

    int idx = 0;
    for (const auto& field : area.fields()) {
      std::string prefix = absl::Substitute(
          "Area $0 field $1$2:", area.area_id().number(), idx++,
          field.has_name() ? absl::Substitute("($0)", field.name()) : "");
      if (field.has_fixed_capital()) {
        checkGoodsExist(field.fixed_capital(), prefix, errors);
      }
      if (field.has_resources()) {
        checkGoodsExist(field.resources(), prefix, errors);
      }
    }
  }
}

// Checks that POPs have valid IDs and wealth.
void validatePops(const games::setup::proto::GameWorld& world,
                  std::vector<std::string>* errors) {
  for (const auto& pop : world.pops()) {
    if (pop.pop_id() == 0) {
      errors->push_back(
          absl::Substitute("Pop without ID: \"$0\"", pop.DebugString()));
    }
    if (pops.find(pop.pop_id()) != pops.end()) {
      errors->push_back(
          absl::Substitute("Pop ID $0 is not unique", pop.pop_id()));
      continue;
    }
    pops[pop.pop_id()] = pop;
    checkGoodsExist(pop.wealth(), absl::Substitute("Pop unit $0", pop.pop_id()),
                    errors);
    // TODO: Area validation here when issue 9 is fixed.
  }
}

// Checks that connections connect areas that exist, and have nonzero length.
void validateConnections(const games::setup::proto::GameWorld& world,
                         std::vector<std::string>* errors) {
  for (const auto& conn : world.connections()) {
    if (!conn.has_id()) {
      errors->push_back(
          absl::Substitute("Connection without ID: $0", conn.DebugString()));
      continue;
    }
    if (conn.id() < 1) {
      errors->push_back(absl::Substitute("Bad connection ID: $0", conn.id()));
    }
    if (connections.find(conn.id()) != connections.end()) {
      errors->push_back(
          absl::Substitute("Connection ID $0 is not unique", conn.id()));
      continue;
    }
    connections[conn.id()] = conn;
    if (!conn.has_a_area_id() || !conn.has_z_area_id()) {
      errors->push_back(
          absl::Substitute("Connection $0 does not connect", conn.id()));
    } else {
      if (areas.find(conn.a_area_id()) == areas.end()) {
        errors->push_back(
            absl::Substitute("Connection $0 has A end $1, which doesn't exist",
                             conn.id(), conn.a_area_id().DebugString()));
      }
      if (areas.find(conn.z_area_id()) == areas.end()) {
        errors->push_back(
            absl::Substitute("Connection $0 has Z end $1, which doesn't exist",
                             conn.id(), conn.z_area_id().DebugString()));
      }
    }
    if (conn.distance_u() < 1) {
      errors->push_back(absl::Substitute("Connection $0 has bad length $1",
                                         conn.id(), conn.distance_u()));
    }
    if (conn.width_u() < 1) {
      errors->push_back(absl::Substitute("Connection $0 has bad width $1",
                                         conn.id(), conn.width_u()));
    }
  }
}

// Checks that units have locations and cargo that exist.
void validateUnits(const games::setup::proto::GameWorld& world,
                   std::vector<std::string>* errors) {
  for (const auto& unit : world.units()) {
    if (!unit.has_unit_id()) {
      errors->push_back(
          absl::Substitute("Unit without ID: \"$0\"", unit.DebugString()));
      continue;
    }
    const auto& unit_id = unit.unit_id();
    if (unit_id.has_type()) {
      errors->push_back("Field 'type' is deprecated in unit ID");
      continue;
    }
    if (!unit_id.has_kind() || !unit_id.has_number()) {
      errors->push_back(
          absl::Substitute("Bad unit ID: $0", unit.DebugString()));
      continue;
    }
    if (template_map.find(unit_id.kind()) == template_map.end()) {
      errors->push_back(absl::Substitute("Unit {$0, $1} has bad kind",
                                         unit_id.kind(), unit_id.number()));
      continue;
    }
    if (unit_map.find(unit_id) != unit_map.end()) {
      errors->push_back(absl::Substitute("Unit {$0, $1} is not unique",
                                         unit_id.kind(), unit_id.number()));
      continue;
    }
    unit_map[unit_id] = unit;
    checkGoodsExist(
        unit.resources(),
        absl::Substitute("Unit {$0, $1}", unit_id.kind(), unit_id.number()),
        errors);
    const auto& location = unit.location();
    if (!location.has_a_area_id()) {
      errors->push_back(absl::Substitute("Unit {$0, $1} has no location",
                                         unit_id.kind(), unit_id.number()));
    } else if (areas.find(location.a_area_id()) == areas.end()) {
      errors->push_back(absl::Substitute(
          "Unit {$0, $1} has nonexistent location $2", unit_id.kind(),
          unit_id.number(), location.a_area_id().DebugString()));
    }
    if (location.has_connection_id()) {
      uint64 conn_id = location.connection_id();
      if (connections.find(conn_id) == connections.end()) {
        errors->push_back(
            absl::Substitute("Unit {$0, $1} has nonexistent connection $2",
                             unit_id.kind(), unit_id.number(), conn_id));
      } else {
        const auto& conn = connections[conn_id];
        if (conn.a_area_id() != location.a_area_id() &&
            conn.z_area_id() != location.a_area_id()) {
          errors->push_back(
              absl::Substitute("Unit {$0, $1} is in connection $2 which does "
                               "not connect source $3",
                               unit_id.kind(), unit_id.number(), conn_id,
                               location.a_area_id().DebugString()));
        }
      }
    }
  }
}

void clear() {
  goods.clear();
  areas.clear();
  templates.clear();
  connections.clear();
  pops.clear();
  unit_map.clear();
}

} // namespace

void RegisterValidator(const std::string& key, Validator v) {
  validator_map[key] = v;
}

std::vector<std::string> Validate(const games::setup::proto::Scenario& scenario,
                                  const games::setup::proto::GameWorld& world) {
  clear();
  std::vector<std::string> errors;
  checkGoods(scenario, &errors);
  checkAutoProduction(scenario, &errors);
  checkProduction(scenario, &errors);
  checkConsumption(scenario, &errors);
  checkTemplates(scenario, &errors);

  validateAreas(world, &errors);
  validatePops(world, &errors);
  validateConnections(world, &errors);
  validateUnits(world, &errors);

  for (const auto& it : validator_map) {
    const auto& key = it.first;
    auto extra_errors = it.second(world);
    for (const auto& extra : extra_errors) {
      errors.push_back(absl::Substitute("$0 : $1", key, extra));
    }
  }

  return errors;
}

namespace optional {

std::vector<std::string>
UnitFactions(const games::setup::proto::GameWorld& world) {
  std::vector<std::string> errors;
  if (world.factions_size() < 1) {
    errors.push_back("Zero factions");
  }

  std::unordered_set<util::proto::ObjectId> factions;
  for (const auto& faction : world.factions()) {
    if (!faction.has_faction_id()) {
      errors.push_back(absl::Substitute("Faction without faction_id: $0",
                                        faction.DebugString()));
      continue;
    }
    if (factions.find(faction.faction_id()) != factions.end()) {
      errors.push_back(absl::Substitute(
          "Duplicate faction ID $0",
          util::objectid::DisplayString(faction.faction_id())));
      continue;
    }
    factions.insert(faction.faction_id());
  }

  for (const auto& unit : world.units()) {
    if (!unit.has_faction_id()) {
      errors.push_back(
          absl::Substitute("Unit $0 has no faction ID",
                           util::objectid::DisplayString(unit.unit_id())));
      continue;
    }
    if (factions.find(unit.faction_id()) == factions.end()) {
      errors.push_back(
          absl::Substitute("Unit $0 belongs to unknown faction $1",
                           util::objectid::DisplayString(unit.unit_id()),
                           util::objectid::DisplayString(unit.faction_id())));
    }
  }

  return errors;
}


} // namespace optional
} // namespace validation
} // namespace setup
} // namespace games
