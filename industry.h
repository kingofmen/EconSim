// Class to organise production chains.

#include "industry/proto/industry.pb.h"
#include "market/proto/goods.pb.h"

namespace industry {

class Progress : public proto::Progress {
public:
  // Does not take ownership of the production proto.
  explicit Progress(const proto::Production *prod) : production_(prod) {}
  ~Progress() = default;

  void PerformStep(market::proto::Container *inputs,
                   market::proto::Container *outputs, int variant_index = 0);

  bool Complete() const;

private:
  const proto::Production *production_;
};

} // namespace industry
