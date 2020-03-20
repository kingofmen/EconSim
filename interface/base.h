#ifndef INTERFACE_INTERFACE_H
#define INTERFACE_INTERFACE_H

#include "interface/proto/config.pb.h"
#include "SDL_keyboard.h"
#include "util/status/status.h"

namespace interface {

// Receiver is an abstract class for receiving user actions.
class Receiver {
 public:
   Receiver() = default;
   ~Receiver() = default;

  // General methods.
  // Quit signal.
  virtual void QuitToDesktop() = 0;

  // SDL methods.
  // Key release.
  virtual void HandleKeyRelease(const SDL_Keysym& keysym) = 0;
};

// Base is an abstract class exposing a minimal set of interactions.
// Implementations may store state.
class Base {
 public:
   Base() = default;
   ~Base() = default;

   virtual util::Status Initialise(const interface::proto::Config& config) = 0;
   virtual void Cleanup() = 0;
   virtual void EventLoop() = 0;
   void SetReceiver(Receiver* c) { receiver_ = c; }

 protected:
  Receiver* receiver_;
};

}  // namespace interface

#endif

  
