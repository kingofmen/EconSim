#ifndef INTERFACE_INTERFACE_H
#define INTERFACE_INTERFACE_H

#include "interface/proto/config.pb.h"

namespace interface {

// Receiver is an abstract class for receiving user actions.
class Receiver {
 public:
   Receiver() = default;
   ~Receiver() = default;
};

// Base is an abstract class exposing a minimal set of interactions.
// Implementations may store state.
class Base {
 public:
   Base() = default;
   ~Base() = default;

   virtual void Initialise(const interface::proto::Config& config) = 0;
   virtual void Cleanup() = 0;
   void SetReceiver(Receiver* c) { receiver_ = c; }

 protected:
  Receiver* receiver_;
};

}  // namespace interface

#endif

  
