// Class to represent a unit of population.
#ifndef POPULATION_POPUNIT_H
#define POPULATION_POPUNIT_H

#include <unordered_map>

#include "population/proto/population.pb.h"

using google::protobuf::uint64;

namespace population {

class PopUnit : proto::PopUnit {
 public:
  PopUnit();

  void BirthAndDeath();
  static PopUnit* GetPopId(uint64 id) {
    return id_to_pop_map_[id];
  }
  static uint64 NewPopId();
 private:
  static std::unordered_map<uint64, PopUnit*> id_to_pop_map_;
};

} // namespace population

#endif
