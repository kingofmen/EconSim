// Class to organise production chains.

#include "market/proto/goods.pb.h"

namespace industry {

class Production {
 public:
  Production() = default;
  ~Production() = default;

 private:
  market::proto::Container inputs_;
};

} // namespace industry
