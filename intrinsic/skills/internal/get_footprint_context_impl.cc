// Copyright 2023 Intrinsic Innovation LLC

#include "intrinsic/skills/internal/get_footprint_context_impl.h"

#include <memory>
#include <string>
#include <utility>

#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "intrinsic/icon/release/status_helpers.h"
#include "intrinsic/skills/proto/equipment.pb.h"
#include "intrinsic/world/objects/frame.h"
#include "intrinsic/world/objects/kinematic_object.h"
#include "intrinsic/world/objects/object_world_ids.h"
#include "intrinsic/world/objects/world_object.h"

namespace intrinsic {
namespace skills {

using ::intrinsic::world::ObjectWorldClient;

absl::StatusOr<world::KinematicObject>
GetFootprintContextImpl::GetKinematicObjectForEquipment(
    absl::string_view equipment_name) {
  INTRINSIC_ASSIGN_OR_RETURN(
      const intrinsic_proto::skills::ResourceHandle handle,
      equipment_.GetHandle(equipment_name));
  return object_world().GetKinematicObject(handle);
}

absl::StatusOr<world::WorldObject>
GetFootprintContextImpl::GetObjectForEquipment(
    absl::string_view equipment_name) {
  INTRINSIC_ASSIGN_OR_RETURN(
      const intrinsic_proto::skills::ResourceHandle handle,
      equipment_.GetHandle(equipment_name));
  return object_world().GetObject(handle);
}

absl::StatusOr<world::Frame> GetFootprintContextImpl::GetFrameForEquipment(
    absl::string_view equipment_name, absl::string_view frame_name) {
  INTRINSIC_ASSIGN_OR_RETURN(
      const intrinsic_proto::skills::ResourceHandle handle,
      equipment_.GetHandle(equipment_name));
  return object_world().GetFrame(handle, FrameName(frame_name));
}

}  // namespace skills
}  // namespace intrinsic