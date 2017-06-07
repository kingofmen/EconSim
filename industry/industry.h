// Class to organise production chains.
#ifndef BASE_INDUSTRY_INDUSTRY_H
#define BASE_INDUSTRY_INDUSTRY_H

#include "industry/proto/industry.pb.h"
#include "market/proto/goods.pb.h"
#include "market/market.h"

namespace industry {

class Production : public proto::Production {
public:
  Production() = default;
  explicit Production(const proto::Production& prod) : proto::Production(prod) {}
  ~Production() = default;

  // Returns true if this progress has completed all steps.
  bool Complete(const proto::Progress& progress) const;

  // Returns the current output multiplier for this progress.
  double Efficiency(const proto::Progress& progress) const;

  // Returns the output given the current scale and efficiency.
  market::proto::Container
  ExpectedOutput(const proto::Progress& progress) const;

  // Returns the input multiplier for the given amount of institutional capital.
  double ExperienceEffect(const double institutional_capital) const;

  // Initialises a Progress proto with this production chain.
  proto::Progress MakeProgress(double scale) const;

  // Returns the theoretical maximum scale, assuming all the necessary resources
  // are present.
  double MaxScale() const;

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
                                               const int variant) const;
  market::proto::Container RequiredConsumables(const int step,
                                               const int variant) const;

  // Skips the current step, at a price in efficiency.
  void Skip(proto::Progress* progress) const;
};

} // namespace industry

#endif
