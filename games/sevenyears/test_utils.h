#ifndef GAMES_SEVENYEARS_TEST_UTILS_H
#define GAMES_SEVENYEARS_TEST_UTILS_H

#include "games/industry/industry.h"
#include "games/setup/setup.h"
#include "games/sevenyears/interfaces.h"
#include "games/sevenyears/proto/sevenyears.pb.h"
#include "games/sevenyears/proto/testdata.pb.h"
#include "util/logging/logging.h"
#include "util/proto/object_id.pb.h"
#include "util/proto/object_id.h"
#include "util/status/status.h"

namespace sevenyears {

// Struct to hold golden protos for tests.
struct Golden {
  std::unique_ptr<std::unordered_map<std::string, actions::proto::Plan*>>
      plans_;
  std::unique_ptr<std::unordered_map<
      std::string, sevenyears::testdata::proto::AreaStateList*>>
      area_states_;
  std::unique_ptr<std::unordered_map<
      std::string, sevenyears::testdata::proto::UnitStateList*>>
      unit_states_;

  void Plans();
  void AreaStates();
  void Units();

  // Not-not here to force implicit bool cast.
  bool HasPlans() {return !!(plans_);}
  bool HasAreaStates() {return !!(area_states_);}
  bool HasUnits() {return !!(unit_states_);}
};

// Loads file locations into the config object, intended to be
// passed on to a full SevenYears object.
void PopulateScenarioFiles(const std::string& location,
                           games::setup::proto::ScenarioFiles* config);

// Loads golden protobufs into the provided object.
util::Status LoadGoldens(const std::string& location, Golden* golds);

// Returns a filename for the provided object ID.
const std::string FileTag(const util::proto::ObjectId& obj_id);

// Compares the world state against the golden state for the given stage.
void CheckAreaStatesForStage(const SevenYearsState& got, const Golden& want,
                             int stage);

// Compares world state against golden unit states for the given stage.
void CheckUnitStatesForStage(const SevenYearsState& got, const Golden& want,
                             int stage);

class TestState : public SevenYearsStateImpl {
public:
  TestState() {}

  util::Status Initialise(const games::setup::proto::ScenarioFiles& config);
  util::Status Initialise(const std::string& location);

  const industry::Production&
  ProductionChain(const std::string& name) const override;

  std::vector<const units::Unit*>
  ListUnits(const units::Filter& filter) const override;

  // Loads proto objects and creates a world; intended for loading partial
  // configurations (for example, templates and units only) without creating
  // a runnable SevenYears game.
  util::Status PopulateProtos(const games::setup::proto::ScenarioFiles& config);

protected:
  games::setup::proto::GameWorld world_proto_;
  games::setup::proto::Scenario scenario_proto_;
};

}  // namespace sevenyears

#endif
