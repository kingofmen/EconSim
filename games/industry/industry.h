// Class to organise production chains.
#ifndef BASE_INDUSTRY_INDUSTRY_H
#define BASE_INDUSTRY_INDUSTRY_H

#include "games/industry/proto/industry.pb.h"
#include "games/market/market.h"
#include "games/market/proto/goods.pb.h"
#include "util/arithmetic/microunits.h"
#include "util/status/status.h"

namespace industry {

class Production {
public:
  Production() = default;
  explicit Production(const proto::Production& prod) : proto_(prod) {}
  ~Production() = default;

  // Returns true if this progress has completed all steps.
  bool Complete(const proto::Progress& progress) const;

  // Returns the current output multiplier in micro-units for this progress.
  micro::Measure EfficiencyU(const proto::Progress& progress) const;

  // Returns the output given the current scale and efficiency.
  market::proto::Container
  ExpectedOutput(const proto::Progress& progress) const;

  // Returns the input multiplier in micro-units for the given amount of
  // institutional capital.
  micro::Measure
  ExperienceEffectU(const micro::Measure institutional_capital_u) const;

  // Initialises a Progress proto with this production chain.
  proto::Progress MakeProgress(micro::Measure scale_u) const;

  // Returns the theoretical maximum scale in micro-units, assuming all the
  // necessary resources are present.
  micro::Measure MaxScaleU() const;

  // Increments the step if possible; otherwise returns false. Takes consumables
  // and movable capital from inputs, and resources from raw_materials; puts
  // movable capital into used_capital. If the process completes, fills outputs
  // with the products.
  util::Status PerformStep(const market::proto::Container& fixed_capital,
                           const micro::Measure institutional_capital,
                           const int variant_index,
                           market::proto::Container* inputs,
                           market::proto::Container* raw_materials,
                           market::proto::Container* outputs,
                           market::proto::Container* used_capital,
                           proto::Progress* progress) const;

  // Returns true if fixed_capital, inputs, and raw_materials respectively
  // contain enough goods to satisfy the capital, consumables, and resource
  // requirements of the current step in progress, using variant_index. Fills
  // the three needed_foo Containers (which may not be null) with the required
  // goods.
  bool StepPossible(const market::proto::Container& fixed_capital,
                    const market::proto::Container& inputs,
                    const market::proto::Container& raw_materials,
                    const proto::Progress& progress,
                    const micro::Measure institutional_capital_u,
                    const int variant_index,
                    market::proto::Container* needed_capital,
                    market::proto::Container* needed_inputs,
                    market::proto::Container* needed_raw_material) const;

  // Returns the consumables needed for the next step in the process assuming
  // variant is used, with the scale given by progress.
  market::proto::Container RequiredConsumables(const proto::Progress& progress,
                                               const int variant) const;

  // Skips the current step, at a price in efficiency.
  void Skip(proto::Progress* progress) const;

  const proto::Production* Proto() const { return &proto_; }
  proto::Production* Proto() { return &proto_; }

  const std::string& get_name() const { return proto_.name(); }
  const proto::ProductionStep& get_step(unsigned int s) const {
    return proto_.steps(s);
  }
  unsigned int num_steps() const { return proto_.steps_size(); }

private:
  market::proto::Container RequiredConsumables(const int step,
                                               const int variant) const;

  proto::Production proto_;
};

} // namespace industry

#endif
