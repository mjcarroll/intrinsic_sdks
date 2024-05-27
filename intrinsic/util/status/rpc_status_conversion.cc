// Copyright 2023 Intrinsic Innovation LLC

#include "intrinsic/util/status/rpc_status_conversion.h"

#include <string>

#include "absl/status/status.h"
#include "absl/strings/cord.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/any.pb.h"
#include "google/rpc/status.pb.h"

namespace intrinsic {

google::rpc::Status SaveStatusAsRpcStatus(const absl::Status& status) {
  google::rpc::Status ret;
  ret.set_code(static_cast<int>(status.code()));
  ret.set_message(std::string(status.message()));
  status.ForEachPayload(
      [&](absl::string_view type_url, const absl::Cord& payload) {
        google::protobuf::Any* any = ret.add_details();
        any->set_type_url(std::string(type_url));
        any->set_value(std::string(payload));
      });
  return ret;
}

absl::Status MakeStatusFromRpcStatus(const google::rpc::Status& status) {
  if (status.code() == 0) return absl::OkStatus();
  absl::Status ret(static_cast<absl::StatusCode>(status.code()),
                   status.message());
  for (const google::protobuf::Any& detail : status.details()) {
    ret.SetPayload(detail.type_url(), absl::Cord(detail.value()));
  }
  return ret;
}

}  // namespace intrinsic
