#ifndef BASE_POPULATION_CONSUMPTION_H
#define BASE_POPULATION_CONSUMPTION_H

#include "market/proto/goods.pb.h"
#include "population/proto/consumption.pb.h"
#include "util/headers/int_types.h"
#include "util/status/status.h"


namespace consumption {

extern const int kMaxSubstitutables;

// Calculates the optimum consumption point given the prices, as shown in the
// doc, and stores it in result.
util::Status Optimum(const proto::Substitutes& subs,
                     const market::proto::Container& prices,
                     market::proto::Container* result);

// Checks that subs does not violate the constraints derived in the doc.
util::Status Validate(const proto::Substitutes& subs);

}  // namespace consumption

#endif
