// Class to organise production chains.
#ifndef BASE_INDUSTRY_INDUSTRY_H
#define BASE_INDUSTRY_INDUSTRY_H

#include "industry/proto/industry.pb.h"
#include "market/proto/goods.pb.h"

namespace industry {

class Production : public proto::Production {
public:
  Production() = default;
  explicit Production(const proto::Production& prod) : proto::Production(prod) {}
  ~Production() = default;

  // Returns true if this progress has completed all steps.
  bool Complete(const proto::Progress& progress) const;

  // Returns the index of the cheapest variant of step, and stores the cost of
  // its consumables in price, which may not be null.
  int CheapestVariant(const market::proto::Container& prices,
                      const market::proto::Container& capital, const int step,
                      double* price) const;
  int CheapestVariant(const market::proto::Container& prices,
                      const market::proto::Container& capital,
                      const proto::Progress& progress, double* price) const {
    return CheapestVariant(prices, capital, progress.step(), price);
  }

  // Returns the current output multiplier for this progress.
  double Efficiency(const proto::Progress& progress) const;

  // Calculates the expected profit of completing the process, given the prices
  // and capital. Progress may be null.
  double ExpectedProfit(const market::proto::Container& prices,
                        const market::proto::Container& capital,
                        const proto::Progress* progress) const;

  // Returns the input multiplier for the given amount of institutional capital.
  double ExperienceEffect(const double institutional_capital) const;

  // Initialises a Progress proto with this production chain.
  proto::Progress MakeProgress(double scale) const;

  // Increments the step if inputs and fixed_capital contains sufficient
  // consumables and capital to run variant_index.
  void PerformStep(const market::proto::Container &fixed_capital,
                   const double institutional_capital,
                   const int variant_index,
                   market::proto::Container *inputs,
                   market::proto::Container *raw_materials,
                   market::proto::Container *outputs,
                   proto::Progress* progress) const;

  // Returns the consumables needed for the next step in the process assuming
  // variant is used.
  market::proto::Container RequiredConsumables(const proto::Progress& progress,
                                               int variant) const;

  // Skips the current step, at a price in efficiency.
  void Skip(proto::Progress* progress) const;
};

} // namespace industry

#endif
