// Class to organise production chains.

#include "industry/proto/industry.pb.h"
#include "market/proto/goods.pb.h"

namespace industry {

class Production : proto::Production {
 public:
  Production() : current_step_(0) {}
  ~Production() = default;

  void PerformStep(market::proto::Container *inputs,
                   market::proto::Container *outputs);

  bool Complete() const;

 private:
  int current_step_;
};

} // namespace industry
