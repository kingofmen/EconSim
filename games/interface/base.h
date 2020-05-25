#ifndef GAMES_INTERFACE_INTERFACE_H
#define GAMES_INTERFACE_INTERFACE_H

#include "games/interface/proto/config.pb.h"
#include "SDL_keyboard.h"
#include "util/status/status.h"
#include "util/proto/object_id.pb.h"

namespace interface {

// Wrapper struct for mouse presses.
struct MouseClick {
  enum Button {
    MB_LEFT,
    MB_MIDDLE,
    MB_RIGHT,
    MB_X1,
    MB_X2
  };
  Button button_;
  int xcoord_;
  int ycoord_;
  int clicks_;
  // If false, button was pressed.
  bool released_;
};

// Receiver is an abstract class for receiving user actions.
class Receiver {
 public:
   Receiver() = default;
   ~Receiver() = default;

  // General methods.
  // Quit signal.
  virtual void QuitToDesktop() = 0;

  // Mouse methods.
  virtual void HandleMouseEvent(const MouseClick& mc) = 0;

  // SDL methods.
  // Key release.
  // TODO: I should probably have my own key-event struct like the one
  // above for mouse events, but oof, that's a lot of boilerplate.
  virtual void HandleKeyRelease(const SDL_Keysym& keysym) = 0;

  virtual void SelectObject(const util::proto::ObjectId& object_id) = 0;
};

// Base is an abstract class exposing a minimal set of interactions.
// Implementations may store state.
class Base {
public:
  Base() = default;
  ~Base() = default;

  virtual util::Status
  Initialise(const games::interface::proto::Config& config) = 0;
  virtual void Cleanup() = 0;
  virtual void EventLoop() = 0;
  void SetReceiver(Receiver* c) { receiver_ = c; }
  virtual void DisplayUnits(const std::vector<util::proto::ObjectId>& ids) = 0;

protected:
  Receiver* receiver_;
};

}  // namespace interface

#endif

  
