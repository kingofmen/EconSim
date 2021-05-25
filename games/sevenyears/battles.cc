#include "games/sevenyears/battles.h"

namespace sevenyears {

void SeaMoveObserver::Listen(const geography::Connection::Movement& movement) {}
void LandMoveObserver::Listen(const geography::Connection::Movement& movement) {}

} // namespace sevenyears
