#ifndef GAMES_SEVENYEARS_BATTLES_H
#define GAMES_SEVENYEARS_BATTLES_H

#include <unordered_map>
#include <vector>

#include "games/geography/connection.h"
#include "games/units/unit.h"
#include "util/proto/object_id.pb.h"

namespace sevenyears {

// Bucket to hold units that meet at a specified point,
// presumably to do battle.
struct Encounter {
  micro::uMeasure point_u;
  util::proto::ObjectId connection_id;
  std::unordered_map<util::proto::ObjectId, std::vector<units::Unit*>> armies;
};

struct BattleResult {
  std::vector<units::Unit*> victors;
  std::vector<units::Unit*> defeated;
  micro::Measure margin_u;
  micro::Measure point_u;
  util::proto::ObjectId connection_id;
};

// Abstract base class for resolving battles.
class BattleResolver {
 public:
  virtual std::vector<BattleResult> Resolve(Encounter& encounter) = 0;
};

// Default implementation.
class DefaultBattleResolver : public BattleResolver {
public:
   std::vector<BattleResult> Resolve(Encounter& encounter) override;

private:
  BattleResult fight(std::vector<units::Unit*> armyOne,
                     std::vector<units::Unit*> armyTwo);
};

// Class to determine whether a unit moving at sea encounters
// any enemies and send such encounters to a resolver.
class SeaMoveObserver : public geography::Connection::Listener {
 public:
  // Check for interceptions by enemy warships.
  void Listen(const geography::Connection::Movement& movement) override;

  // Resolve at-sea encounters.
  void Battle(BattleResolver& resolver);

  // Clear the cache.
  void Clear();
};

// Class to observe land movements and resolve any battles that occur.
class LandMoveObserver : public geography::Connection::Listener {
 public:
  // Accumulate units for possible battle.
  void Listen(const geography::Connection::Movement& movement) override;

  // Find and resolve battles.
   std::vector<BattleResult> Battle(BattleResolver& resolver);

  // Clear the cache.
  void Clear();
  
 private:
  // Map from connection IDs to movements.
   std::unordered_map<util::proto::ObjectId,
                      std::vector<geography::Connection::Movement>>
       traversals_;
};

// Retreats losing units and starts sieges.
void ApplyBattleOutcome(const BattleResult& battle);

// Returns the distance at which the two movements meet, which may be
// out of range of the actual movements. Exposed for testing.
micro::Measure Interception(const geography::Connection::Movement& movement,
                            const geography::Connection::Movement& otherMove,
                            micro::Measure length_u);

} // namespace sevenyears

#endif
