#include "games/colony/interface/factory.h"

#include "games/colony/controller/controller.h"
#include "games/colony/interface/impl/text_interface.h"

namespace interface {
namespace factory {

interface::UserInterface* NewInterface(controller::GameControl* c) {
  return new interface::text::TextInterface(c);
}

}  // namespace factory
}  // namespace interface
