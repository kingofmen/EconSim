#include "game/validation/validation.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "absl/strings/substitute.h"
#include "market/proto/goods.pb.h"
#include "util/headers/int_types.h"
#include "util/proto/object_id.h"

std::unordered_map<std::string, market::proto::TradeGood> goods;
std::unordered_map<uint64, geography::proto::Area> areas;
std::unordered_map<uint64, units::proto::Template> templates;
std::unordered_map<uint64, geography::proto::Connection> connections;
std::unordered_map<uint64, population::proto::PopUnit> pops;
std::unordered_map<util::proto::ObjectId, units::proto::Unit> unit_map;

namespace game {
namespace validation {
namespace {

// Validates that goods have sensible values.
void checkGoods(const game::proto::Scenario& scenario,
                std::vector<std::string>* errors) {
  for (const market::proto::TradeGood& good : scenario.trade_goods()) {
    goods.emplace(good.name(), good);
    if (0 >= good.bulk_u()) {
      errors->push_back(
          absl::Substitute("$0 has bad bulk $1", good.name(), good.bulk_u()));
    }
    if (0 >= good.weight_u()) {
      errors->push_back(absl::Substitute("$0 has bad weight $1", good.name(),
                                        good.weight_u()));
    }
    if (0 > good.decay_rate_u()) {
      errors->push_back(absl::Substitute("$0 has bad decay rate $1", good.name(),
                                        good.decay_rate_u()));
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
      errors->push_back(absl::Substitute("$0: Good $1 does not exist.", prefix, name));
    }
  }
}

// Checks that all autoproduction output exists.
void checkAutoProduction(const game::proto::Scenario& scenario,
                         std::vector<std::string>* errors) {
  for (const population::proto::AutoProduction& ap : scenario.auto_production()) {
    checkGoodsExist(ap.output(), "Auto production", errors);
  }
}

// Checks that all production inputs, outputs, and capital exist.
void checkProduction(const game::proto::Scenario& scenario,
                         std::vector<std::string>* errors) {
  for (const auto& prod : scenario.production_chains()) {
    std::string name = prod.name();
    if (name == "") {
      errors->push_back(absl::Substitute("Chain without name: $0", prod.DebugString()));
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
void checkConsumption(const game::proto::Scenario& scenario,
                      std::vector<std::string>* errors) {
  for (const auto& con : scenario.consumption()) {
    std::string name = con.name();
    if (name == "") {
      errors->push_back(absl::Substitute("Consumption without name: $0", con.DebugString()));
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
void checkTemplates(const game::proto::Scenario& scenario,
                    std::vector<std::string>* errors) {
  for (const auto& temp : scenario.unit_templates()) {
    if (!temp.has_id()) {
      errors->push_back(
          absl::Substitute("Template without ID: $0", temp.DebugString()));
      continue;
    }
    templates[temp.id()] = temp;
  }
}

// Checks that areas have valid IDs.
void validateAreas(const game::proto::GameWorld& world,
                   std::vector<std::string>* errors) {
  for (const auto& area : world.areas()) {
    if (!area.has_id()) {
      errors->push_back(
          absl::Substitute("Area without ID: \"$0\"", area.DebugString()));
    } else if (area.id() < 1) {
      errors->push_back(absl::Substitute("Bad area ID: $0", area.id()));
    } else if (areas.find(area.id()) != areas.end()) {
        errors->push_back(absl::Substitute("Area ID $0 is not unique", area.id()));
    } else
      areas[area.id()] = area;
  }
}

// Checks that POPs have valid IDs and wealth.
void validatePops(const game::proto::GameWorld& world,
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
void validateConnections(const game::proto::GameWorld& world,
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
    if (!conn.has_a() || !conn.has_z()) {
      errors->push_back(
          absl::Substitute("Connection $0 does not connect", conn.id()));
    } else {
      if (areas.find(conn.a()) == areas.end()) {
        errors->push_back(
            absl::Substitute("Connection $0 has A end $1, which doesn't exist",
                             conn.id(), conn.a()));
      }
      if (areas.find(conn.z()) == areas.end()) {
        errors->push_back(
            absl::Substitute("Connection $0 has Z end $1, which doesn't exist",
                             conn.id(), conn.z()));
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
void validateUnits(const game::proto::GameWorld& world,
                   std::vector<std::string>* errors) {
  for (const auto& unit : world.units()) {
    if (!unit.has_unit_id()) {
      errors->push_back(absl::Substitute("Unit without ID: \"$0\"", unit.DebugString()));
      continue;
    }
    const auto& unit_id = unit.unit_id();
    if (!unit_id.has_type() || !unit_id.has_number()) {
      errors->push_back(absl::Substitute("Bad unit ID: $0", unit.DebugString()));
      continue;
    }
    if (templates.find(unit_id.type()) == templates.end()) {
      errors->push_back(absl::Substitute("Unit {$0, $1} has bad type",
                                         unit_id.type(), unit_id.number()));
    }
    if (unit_map.find(unit_id) != unit_map.end()) {
      errors->push_back(absl::Substitute("Unit {$0, $1} is not unique",
                                         unit_id.type(), unit_id.number()));
      continue;
    }
    unit_map[unit_id] = unit;
    checkGoodsExist(
        unit.resources(),
        absl::Substitute("Unit {$0, $1}", unit_id.type(), unit_id.number()),
        errors);
    const auto& location = unit.location();
    if (!location.has_source_area_id()) {
      errors->push_back(absl::Substitute("Unit {$0, $1} has no location",
                                         unit_id.type(), unit_id.number()));
    } else if (areas.find(location.source_area_id()) == areas.end()) {
      errors->push_back(absl::Substitute(
          "Unit {$0, $1} has nonexistent location $2", unit_id.type(),
          unit_id.number(), location.source_area_id()));
    }
    if (location.has_connection_id()) {
      uint64 conn_id = location.connection_id();
      if (connections.find(conn_id) == connections.end()) {
        errors->push_back(
            absl::Substitute("Unit {$0, $1} has nonexistent connection $2",
                             unit_id.type(), unit_id.number(), conn_id));
      } else {
        const auto& conn = connections[conn_id];
        if (conn.a() != location.source_area_id() && conn.z() != location.source_area_id()) {
          errors->push_back(
              absl::Substitute("Unit {$0, $1} is in connection $2 which does "
                               "not connect source $3",
                               unit_id.type(), unit_id.number(), conn_id,
                               location.source_area_id()));
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

}  // namespace

std::vector<std::string> Validate(const game::proto::Scenario& scenario,
                                  const game::proto::GameWorld& world) {
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
  
  return errors;
}

}  // namespace validation
}  // namespace game
