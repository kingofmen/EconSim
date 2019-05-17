#ifndef COLONY_INTERFACE_IMPL_TEXT_INTERFACE_H
#define COLONY_INTERFACE_IMPL_TEXT_INTERFACE_H

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "colony/controller/controller.h"
#include "colony/interface/user_interface.h"
#include "game/game_world.h"
#include "game/proto/game_world.pb.h"
#include "game/setup/proto/setup.pb.h"
#include "src/google/protobuf/stubs/status.h"

namespace interface {
namespace text {

// Text implementation of UserInterface.
class TextInterface : public interface::UserInterface {
 public:
  TextInterface(controller::GameControl* c);
  void IntroScreen() override;

 private:
  std::vector<std::string> display_;
  std::vector<std::vector<std::tuple<int, int>>> colours_;
  bool quit_;
  std::function<void(char)> handler_;
  std::vector<game::setup::proto::ScenarioFiles> scenario_files_;
  std::deque<std::tuple<int, std::string>> messages_;
  game::proto::Scenario scenario_;
  game::proto::GameWorld game_world_;
  std::unique_ptr<game::GameWorld> world_model_;

  template <typename T>
  void addSelection(int x, int y, const std::vector<T>& options,
                    std::function<std::string(const T&)> toString);

  void introHandler(char inp);
  void loadGameHandler(char inp);
  void newGameHandler(char inp);
  void runGameHandler(char inp);

  void gameDisplay();
  void loadGameScreen();
  void mainMenu();
  void newGameScreen();
  
  void awaitInput();
  void clear();
  void clearLine(int line);
  void drawMessageBox();
  void drawWorld();
  void message(int mask, const std::string& error);
  void flip() const;
  google::protobuf::util::Status
  loadScenario(const game::setup::proto::ScenarioFiles& setup);
  void loadWorld(const std::string& filename);
  void output(int x, int y, int mask, const std::string& words);
};

}  // namespace text
}  // namespace interface

#endif
