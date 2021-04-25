#ifndef GAMES_SEVENYEARS_SEVENYEARS_H
#define GAMES_SEVENYEARS_SEVENYEARS_H

#include <string>
#include <vector>

#include "games/interface/base.h"
#include "games/setup/proto/setup.pb.h"
#include "games/sevenyears/action_cost_calculator.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/merchant_ship_ai.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "util/proto/object_id.pb.h"
#include "util/status/status.h"

namespace sevenyears {

// Class for running actual game mechanics.
class SevenYears : public SevenYearsStateImpl, public interface::StateFetcher {
public:
  SevenYears() : dirtyGraphics_(true) {}
  ~SevenYears() {}

  util::Status LoadScenario(const games::setup::proto::ScenarioFiles& setup);
  void NewTurn();
  void UpdateGraphicsInfo(interface::Base* gfx);
  util::Status InitialiseAI();

  const sevenyears::proto::AreaState&
  AreaState(const util::proto::ObjectId& area_id) const override;

  const industry::Production&
  ProductionChain(const std::string& name) const override;

  // Load the state of object_id into proto, if it exists.
  void Fetch(const util::proto::ObjectId& object_id,
             google::protobuf::Message* proto);

  sevenyears::proto::AreaState*
  mutable_area_state(const util::proto::ObjectId& area_id) override;

private:
  // Moves units, updating their plans if needed.
  void moveUnits();
  void runAreaProduction(proto::AreaState* area_state, geography::Area* area);
  void runEuropeanTrade(proto::AreaState* area_state, geography::Area* area);
  std::vector<std::string>
  validation(const games::setup::proto::GameWorld& world);

  // Executors.
  util::Status doEuropeanTrade(const actions::proto::Step& step,
                               units::Unit* unit);
  util::Status loadShip(micro::Measure fraction_u,
                        const actions::proto::Step& step, units::Unit* unit);
  util::Status offloadCargo(micro::Measure fraction_u,
                            const actions::proto::Step& step, units::Unit* unit,
                            market::proto::Container* amount);

  bool dirtyGraphics_;
  std::unordered_map<std::string, industry::Production> production_chains_;
  std::unordered_map<util::proto::ObjectId, sevenyears::proto::AreaState>
      area_states_;
  std::unique_ptr<sevenyears::SevenYearsMerchant> merchant_ai_;
  std::unique_ptr<sevenyears::ActionCostCalculator> cost_calculator_;
};

}  // namespace sevenyears

#endif
