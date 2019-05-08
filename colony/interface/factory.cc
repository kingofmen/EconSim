#include "colony/interface/factory.h"

#include "colony/controller/controller.h"
#include "colony/interface/impl/text_interface.h"

namespace interface {
namespace factory {

interface::UserInterface* NewInterface(controller::GameControl* c) {
  return new interface::text::TextInterface(c);
}

}  // namespace factory
}  // namespace interface
