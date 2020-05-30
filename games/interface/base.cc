#include "games/interface/base.h"

namespace interface {

void Base::GetState(const util::proto::ObjectId& object_id,
                    google::protobuf::Message* proto) {
  if (fetcher_ != nullptr) {
    fetcher_->Fetch(object_id, proto);
  }
}

}  // namespace interface
