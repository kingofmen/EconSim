#include "population/popunit.h"

namespace population {

std::unordered_map<uint64, PopUnit*> PopUnit::id_to_pop_map_;
uint64 unused_pop_id = 0;
constexpr int kNumAgeGroups = 7;

PopUnit::PopUnit() {
  set_pop_id(NewPopId());
  for (int i = 0; i < kNumAgeGroups; ++i) {
    add_males(0);
    add_women(0);
  }
  id_to_pop_map[pop_id()] = this;
}

void PopUnit::BirthAndDeath() {}

uint64 PopUnit::NewPopId() {
  return ++unused_pop_id;
}



} // namespace population
