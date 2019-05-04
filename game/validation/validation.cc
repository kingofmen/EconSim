#include "game/validation/validation.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "absl/strings/substitute.h"
#include "market/proto/goods.pb.h"

std::unordered_map<std::string, market::proto::TradeGood> goods;
#include <iostream>
namespace game {
namespace validation {

// Validates that goods have sensible values.
void checkGoods(const game::proto::Scenario& scenario,
                std::vector<std::string>* errors) {
  for (const market::proto::TradeGood& good : scenario.trade_goods()) {
    goods.emplace(good.name(), good);
    std::cout << "Checking " << good.name() << "\n";
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


std::vector<std::string> Validate(const game::proto::Scenario& scenario,
                                  const game::proto::GameWorld& world) {
  std::vector<std::string> errors;
  checkGoods(scenario, &errors);
  checkAutoProduction(scenario, &errors);
  return errors;
}

}  // namespace validation
}  // namespace game
