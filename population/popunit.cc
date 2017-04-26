#include "population/popunit.h"

#include <limits>
#include <list>

#include "geography/geography.h"
#include "industry/industry.h"
#include "market/goods_utils.h"

namespace population {

using industry::proto::Production;

namespace {
uint64 unused_pop_id = 1;
constexpr int kNumAgeGroups = 7;
constexpr char kExpectedProfit[] = "expected_profit";
constexpr char kStandardDeviation[] = "standard_deviation";
constexpr char kCorrelation[] = "correlation";
constexpr char kSubsistence[] = "subsistence";

market::proto::Container TotalNeeded(const proto::ConsumptionPackage& package,
                                     int size) {
  auto needed = package.food().consumed();
  needed += package.food().capital();
  needed += package.shelter().consumed();
  needed += package.shelter().capital();
  needed += package.culture().consumed();
  needed += package.culture().capital();
  needed *= size;
  return needed;
}

} // namespace

std::unordered_map<uint64, PopUnit*> PopUnit::id_to_pop_map_;

PopUnit::PopUnit() {
  set_pop_id(NewPopId());
  for (int i = 0; i < kNumAgeGroups; ++i) {
    add_males(0);
    add_women(0);
  }
  id_to_pop_map_[pop_id()] = this;
}

PopUnit::PopUnit(const proto::PopUnit& proto) : proto::PopUnit(proto) {
  if (id_to_pop_map_[pop_id()] != nullptr) {
    // TODO: Error here.
  }
  id_to_pop_map_[pop_id()] = this;
}

void PopUnit::AutoProduce(
    const std::vector<const proto::AutoProduction*>& production,
    const market::proto::Container& prices) {
  const proto::AutoProduction* best_prod = nullptr;
  double best_price = 0;
  for (const auto* p : production) {
    if (!(tags() > p->required_tags())) {
      continue;
    }
    double curr_price = p->output() * prices;
    if (curr_price < best_price) {
      continue;
    }
    best_price = curr_price;
    best_prod = p;
  }
  if (best_prod == nullptr) {
    return;
  }
  *mutable_wealth() += best_prod->output();
}

void PopUnit::BirthAndDeath() {}

const proto::ConsumptionPackage*
PopUnit::CheapestPackage(const proto::ConsumptionLevel& level,
                         const market::proto::Container& prices) const {
  const proto::ConsumptionPackage* best_package = nullptr;
  double best_price = std::numeric_limits<double>::max();
  int size = GetSize();
  for (const auto& package : level.packages()) {
    if (!(tags() > package.required_tags())) {
      continue;
    }

    if (wealth() > TotalNeeded(package, size)) {
      double curr_price = prices * package.food().consumed();
      curr_price += prices * package.shelter().consumed();
      curr_price += prices * package.culture().consumed();
      if (curr_price < best_price) {
        best_package = &package;
        best_price = curr_price;
      }
    }
  }
  return best_package;
}

bool PopUnit::Consume(const proto::ConsumptionLevel& level,
                      const market::proto::Container& prices) {
  const proto::ConsumptionPackage* best_package = CheapestPackage(level, prices);
  if (best_package == nullptr) {
    return false;
  }

  auto& resources = *mutable_wealth();
  int size = GetSize();
  resources -= best_package->food().consumed() * size;
  resources -= best_package->shelter().consumed() * size;
  resources -= best_package->culture().consumed() * size;

  *mutable_tags() += best_package->tags();
  *mutable_tags() += level.tags();
  return true;
}

void PopUnit::DecayWealth(const market::proto::Container& decay_rates) {
  *mutable_wealth() *= decay_rates;
  market::CleanContainer(mutable_wealth());
}

int PopUnit::GetSize() const {
  int size = 0;
  for (const auto men : males()) {
    size += men;
  }
  for (const auto females : women()) {
    size += females;
  }
  return size;
}

void PopUnit::CandidateHeuristics(
    const market::proto::Container& prices, const ProductionMap& /*chains*/,
    const std::vector<ProductionCandidate>& /*selected*/,
    ProductionCandidate* candidate) {
  market::SetAmount(
      kExpectedProfit,
      candidate->process->ExpectedProfit(prices, candidate->progress),
      &candidate->heuristics);
}

void PopUnit::PossibleProduction(
    const ProductionMap& chains,
    const std::vector<geography::proto::Field*>& fields,
    std::list<ProductionCandidate>* candidates) {
  for (auto* field : fields) {
    if (field->has_production()) {
      const industry::Production* production =
          chains.at(field->production().name());
      candidates->emplace_back(field, production, field->mutable_production());
      continue;
    }
    for (const auto& chain : chains) {
      const industry::Production* production = chain.second;
      if (!geography::HasLandType(*field, *production)) {
        continue;
      }
      if (!geography::HasRawMaterials(*field, *production)) {
        continue;
      }
      if (!geography::HasFixedCapital(*field, *production)) {
        continue;
      }
      candidates->emplace_back(field, production, nullptr);
    }
  }
}

void PopUnit::SelectProduction(const market::proto::Container& prices,
                               const ProductionMap& chains,
                               std::list<ProductionCandidate>* candidates,
                               std::vector<ProductionCandidate>* selected) {
  static market::proto::Container weights;
  if (!market::Contains(weights, kExpectedProfit)) {
    market::SetAmount(kExpectedProfit, 1, &weights);
    market::SetAmount(kStandardDeviation, -1, &weights);
    market::SetAmount(kCorrelation, -1, &weights);
    market::SetAmount(kSubsistence, 1, &weights);
  }
  std::unordered_set<geography::proto::Field*> fields_used;
  while (!candidates->empty()) {
    for (auto& candidate : *candidates) {
      CandidateHeuristics(prices, chains, *selected, &candidate);
    }
    candidates->sort(
        [](const ProductionCandidate& a, const ProductionCandidate& b) {
          return a.heuristics * weights < b.heuristics * weights;
        });
    selected->push_back(candidates->back());
    fields_used.insert(candidates->back().target);
    candidates->pop_back();
    candidates->remove_if([&fields_used](const ProductionCandidate& cand) {
      return fields_used.count(cand.target);
    });
  }
}

void PopUnit::Produce(const market::proto::Container& prices,
                      const ProductionMap& chains,
                      const std::vector<geography::proto::Field*>& fields) {
  std::list<ProductionCandidate> candidates;
  PossibleProduction(chains, fields, &candidates);
  std::vector<ProductionCandidate> selected;
  SelectProduction(prices, chains, &candidates, &selected);

  for (auto& candidate : selected) {
    if (candidate.progress == nullptr) {
      *candidate.target->mutable_production() =
          candidate.process->MakeProgress(1.0);
      candidate.progress = candidate.target->mutable_production();
    }
    candidate.process->PerformStep(candidate.target->fixed_capital(), 0.0, 0,
                                   mutable_wealth(),
                                   candidate.target->mutable_resources(),
                                   mutable_wealth(), candidate.progress);
    if (candidate.process->Complete(*candidate.progress)) {
      candidate.target->clear_production();
    }
  }
}

uint64 PopUnit::NewPopId() { return ++unused_pop_id; }

} // namespace population
