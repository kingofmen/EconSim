// Class to organise production chains.

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

  // Returns the current output multiplier for this progress.
  double Efficiency(const proto::Progress& progress) const;

  // Returns the input multiplier for the given amount of institutional capital.
  double ExperienceEffect(const double institutional_capital) const;

  // Initialises a Progress proto with this production chain.
  proto::Progress MakeProgress(double scale);

  // Increments the step if inputs and fixed_capital contains sufficient
  // consumables and capital to run variant_index.
  void PerformStep(const market::proto::Container &fixed_capital,
                   const double institutional_capital,
                   const int variant_index,
                   market::proto::Container *inputs,
                   market::proto::Container *raw_materials,
                   market::proto::Container *outputs,
                   proto::Progress* progress);

  // Skips the current step, at a price in efficiency.
  void Skip(proto::Progress* progress);
};

} // namespace industry
