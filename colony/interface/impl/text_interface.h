#ifndef COLONY_INTERFACE_IMPL_TEXT_INTERFACE_H
#define COLONY_INTERFACE_IMPL_TEXT_INTERFACE_H

#include <functional>
#include <string>
#include <vector>

#include "colony/controller/controller.h"
#include "colony/interface/user_interface.h"

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

  void introHandler(char inp);
  void newGameHandler(char inp);

  void mainMenu();
  void newGameScreen();
  
  void awaitInput();
  void output(int x, int y, const std::string& words);
};

}  // namespace text
}  // namespace interface

#endif
