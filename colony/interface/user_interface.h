#ifndef COLONY_INTERFACE_USER_INTERFACE_H
#define COLONY_INTERFACE_USER_INTERFACE_H

namespace interface {

// Abstract base class for all user interfaces.
class UserInterface {
 public:
  virtual void IntroScreen() = 0;
};

}  // namespace interface

#endif
