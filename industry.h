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

  // Increments the step if inputs contains sufficient consumables and capital.
  void PerformStep(market::proto::Container *inputs,
                   market::proto::Container *outputs, int variant_index = 0);

private:
  const proto::Production *production_;
};

} // namespace industry
