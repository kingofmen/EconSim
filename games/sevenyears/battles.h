#ifndef GAMES_SEVENYEARS_BATTLES_H
#define GAMES_SEVENYEARS_BATTLES_H

#include "games/geography/connection.h"

namespace sevenyears {

// Class to determine whether a unit moving at sea encounters
// any enemies, and if so, the result.
class SeaMoveObserver : public geography::Connection::Listener {
  void Listen(const geography::Connection::Movement& movement) override;
};

// Class to observe land movements and resolve any battles that occur.
class LandMoveObserver : public geography::Connection::Listener {
  void Listen(const geography::Connection::Movement& movement) override;
};

} // namespace sevenyears

#endif
