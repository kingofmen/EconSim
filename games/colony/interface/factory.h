#ifndef COLONY_INTERFACE_FACTORY_H
#define COLONY_INTERFACE_FACTORY_H

#include "colony/controller/controller.h"
#include "colony/interface/user_interface.h"

namespace interface {
namespace factory {

interface::UserInterface* NewInterface(controller::GameControl* c);

}  // namespace factory
}  // namespace interface

#endif
