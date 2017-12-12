// Class to organise production chains.
#ifndef BASE_INDUSTRY_INDUSTRY_H
#define BASE_INDUSTRY_INDUSTRY_H

#include "industry/proto/industry.pb.h"
#include "market/market.h"
#include "market/proto/goods.pb.h"

namespace industry {

class Production {
public:
  Production() = default;
  explicit Production(const proto::Production& prod) : proto_(prod) {}
  ~Production() = default;

  // Returns true if this progress has completed all steps.
  bool Complete(const proto::Progress& progress) const;

  // Returns the current output multiplier in micro-units for this progress.
  market::Measure EfficiencyU(const proto::Progress& progress) const;

  // Returns the output given the current scale and efficiency.
  market::proto::Container
  ExpectedOutput(const proto::Progress& progress) const;

  // Returns the input multiplier in micro-units for the given amount of
  // institutional capital.
  market::Measure
  ExperienceEffectU(const market::Measure institutional_capital_u) const;

  // Initialises a Progress proto with this production chain.
  proto::Progress MakeProgress(market::Measure scale_u) const;

  // Returns the theoretical maximum scale in micro-units, assuming all the
  // necessary resources are present.
  market::Measure MaxScaleU() const;

  // Increments the step if inputs and fixed_capital contains sufficient
  // consumables and capital to run variant_index. Takes consumables and movable
  // capital from inputs, and resources from raw_materials; puts movable capital
  // into used_capital. If the process completes, fills outputs with the
  // products. Returns true if the step is completed.
  bool PerformStep(const market::proto::Container& fixed_capital,
                   const market::Measure institutional_capital,
                   const int variant_index, market::proto::Container* inputs,
                   market::proto::Container* raw_materials,
                   market::proto::Container* outputs,
                   market::proto::Container* used_capital,
                   proto::Progress* progress) const;

  // Returns the consumables needed for the next step in the process assuming
  // variant is used.
  market::proto::Container RequiredConsumables(const proto::Progress& progress,
                                               const int variant) const;
  market::proto::Container RequiredConsumables(const int step,
                                               const int variant) const;

  // Skips the current step, at a price in efficiency.
  void Skip(proto::Progress* progress) const;

  const proto::Production* Proto() const { return &proto_; }
  proto::Production* Proto() { return &proto_; }

  const std::string& get_name() const { return proto_.name(); }

private:
  proto::Production proto_;
};

} // namespace industry

#endif
