#include "colony/controller/controller.h"
#include "colony/interface/user_interface.h"
#include "colony/interface/factory.h"

int main(int /*argc*/, char** /*argv*/) {
  interface::UserInterface* ui =
      interface::factory::NewInterface(new controller::GameControl());
  ui->introScreen();
  return 0;
}

