#ifndef COLONY_INTERFACE_IMPL_TEXT_INTERFACE_H
#define COLONY_INTERFACE_IMPL_TEXT_INTERFACE_H

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "games/colony/controller/controller.h"
#include "games/colony/graphics/proto/graphics.pb.h"
#include "games/colony/interface/user_interface.h"
#include "games/colony/interface/proto/actions.pb.h"
#include "games/setup/proto/setup.pb.h"
#include "games/sinews/game_world.h"
#include "util/status/status.h"
#include "util/proto/object_id.pb.h"

namespace interface {
namespace text {

// Text implementation of UserInterface.
class TextInterface : public interface::UserInterface {
 public:
  enum InputArea {
    IA_POP = 0,
    IA_FIELD = 1,
  };

  TextInterface(controller::GameControl* c);
  ~TextInterface();
  void IntroScreen() override;

 private: 
  std::vector<std::string> display_;
  std::vector<std::vector<std::tuple<int, int>>> colours_;
  bool quit_;
  std::function<void(char)> handler_;
  std::vector<games::setup::proto::ScenarioFiles> scenario_files_;
  std::deque<std::tuple<int, std::string>> messages_;
  games::setup::proto::Scenario scenario_;
  games::setup::proto::GameWorld game_world_;
  colony::graphics::proto::WorldGraphics graphics_;
  colony::graphics::proto::Point center_;
  std::unique_ptr<game::GameWorld> world_model_;
  util::proto::ObjectId selected_area_id_;
  int64 selected_area_index_;
  InputArea selected_input_area_;
  uint64 selected_detail_idx_;
  uint64 player_faction_id_;
  std::unordered_map<const geography::proto::Field*, uint64> field_overrides_;
  std::vector<colony::interface::proto::PlayerAction> actions_;
  industry::decisions::FieldMap<industry::decisions::proto::ProductionDecision>
      decisions_;

  template <typename T>
  void addSelection(int x, int y, const std::vector<T>& options,
                    std::function<std::string(const T&)> toString);

  // Input handlers for different game states.
  void introHandler(char inp);
  void loadGameHandler(char inp);
  void newGameHandler(char inp);
  void runGameHandler(char inp);

  // Draw and await input for various game states.
  void gameDisplay();
  void loadGameScreen();
  void mainMenu();
  void newGameScreen();

  void awaitInput();
  void clear();
  void clearLine(int line);

  // Finders for current selections.
  geography::Area* getArea();
  geography::proto::Field* getField();
  void selectArea(int idx);
  
  // Changes which element is displayed in detail.
  void changeDetailIndex(bool pos);

  // Changes an aspect of the currently detail-displayed element; for example,
  // the player-set process for the displayed field.
  void changeCurrentElement(bool pos);
  void changeFieldProcess(bool pos);

  // Mark readiness to advance the world state by a timestep.
  void endPlayerTurn();

  // Draw various bits of the screen.
  void drawFieldDetails(const geography::proto::Field& field, int& line);
  void drawPopDetails(const population::PopUnit& pop, int& line);
  void drawInfoBox();
  void drawMarket(const market::Market& market, int& line);
  void drawMessageBox();
  void drawWorld();
  void message(int mask, const std::string& error);
  void flip() const;

  // Load scenario into memory.
  util::Status loadScenario(const games::setup::proto::ScenarioFiles& setup);
  void loadWorld(const std::string& filename);
  void output(int x, int y, int mask, const std::string& words);
};

}  // namespace text
}  // namespace interface

#endif
