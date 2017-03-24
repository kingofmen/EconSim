// Class to organise production chains.

#include "industry/proto/industry.pb.h"
#include "market/proto/goods.pb.h"

namespace industry {

class Progress : public proto::Progress {
public:
  // Does not take ownership of the production proto.
  explicit Progress(const proto::Production *prod, int scale = 1);
  ~Progress() = default;

  // Returns true if this process has completed all steps.
  bool Complete() const;

  // Returns the current output multiplier for this process.
  double Efficiency() const;

  // Returns the input multiplier for the given amount of institutional capital.
  double ExperienceEffect(const double institutional_capital) const;

  // Increments the step if inputs and fixed_capital contains sufficient
  // consumables and capital to run variant_index.
  void PerformStep(const market::proto::Container &fixed_capital,
                   market::proto::Container *inputs,
                   market::proto::Container *raw_materials,
                   market::proto::Container *outputs,
                   const double institutional_capital = 0,
                   const int variant_index = 0);

  // Skips the current step, at a price in efficiency.
  void Skip();

private:
  const proto::Production *production_;
};

} // namespace industry
