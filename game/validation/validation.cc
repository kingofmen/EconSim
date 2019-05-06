#include "game/validation/validation.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "absl/strings/substitute.h"
#include "market/proto/goods.pb.h"
#include "util/headers/int_types.h"

std::unordered_map<std::string, market::proto::TradeGood> goods;
std::unordered_map<uint64, geography::proto::Area> areas;

namespace game {
namespace validation {

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

// TODO: Implement this when templates are more complete.
void checkTemplates(const game::proto::Scenario& scenario,
                    std::vector<std::string>* errors) {}

// Checks that areas have valid IDs.
void validateAreas(const game::proto::GameWorld& world,
                   std::vector<std::string>* errors) {
  for (const auto& area : world.areas()) {
    if (!area.has_id()) {
      errors->push_back(
          absl::Substitute("Area without ID: \"$0\"", area.DebugString()));
    } else if (area.id() < 1) {
      errors->push_back(absl::Substitute("Bad area ID: $0", area.id()));
    } else {
      areas[area.id()] = area;
    }
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

void validateUnits(const game::proto::GameWorld& world,
                   std::vector<std::string>* errors) {}

std::vector<std::string> Validate(const game::proto::Scenario& scenario,
                                  const game::proto::GameWorld& world) {
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
