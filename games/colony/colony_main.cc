#include "games/colony/controller/controller.h"
#include "games/colony/interface/user_interface.h"
#include "games/colony/interface/factory.h"

int main(int /*argc*/, char** /*argv*/) {
  interface::UserInterface* ui =
      interface::factory::NewInterface(new controller::GameControl());
  ui->IntroScreen();
  return 0;
}

