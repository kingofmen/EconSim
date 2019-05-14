#ifndef COLONY_INTERFACE_IMPL_TEXT_INTERFACE_H
#define COLONY_INTERFACE_IMPL_TEXT_INTERFACE_H

#include <functional>
#include <string>
#include <vector>

#include "colony/controller/controller.h"
#include "colony/interface/user_interface.h"
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
  bool quit_;
  std::function<void(char)> handler_;
  std::vector<game::setup::proto::ScenarioFiles> scenario_files_;
  game::proto::Scenario scenario_;
  game::proto::GameWorld game_world_;

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
  void errorMessage(const std::string& error);
  google::protobuf::util::Status
  loadScenario(const game::setup::proto::ScenarioFiles& setup);
  void loadWorld(const std::string& filename);
  void output(int x, int y, const std::string& words);
};

}  // namespace text
}  // namespace interface

#endif
