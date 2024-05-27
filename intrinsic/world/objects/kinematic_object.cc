// Copyright 2023 Intrinsic Innovation LLC
// Intrinsic Proprietary and Confidential
// Provided subject to written agreement between the parties.

#include "intrinsic/world/objects/kinematic_object.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/algorithm/container.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_join.h"
#include "absl/strings/substitute.h"
#include "intrinsic/eigenmath/types.h"
#include "intrinsic/icon/proto/cart_space_conversion.h"
#include "intrinsic/icon/release/status_helpers.h"
#include "intrinsic/kinematics/types/cartesian_limits.h"
#include "intrinsic/kinematics/types/joint_limits_xd.h"
#include "intrinsic/util/eigen.h"
#include "intrinsic/world/objects/frame.h"
#include "intrinsic/world/objects/object_world_ids.h"
#include "intrinsic/world/objects/transform_node.h"
#include "intrinsic/world/objects/world_object.h"
#include "intrinsic/world/proto/object_world_service.pb.h"

namespace intrinsic {
namespace world {

absl::StatusOr<KinematicObject> KinematicObject::Create(
    intrinsic_proto::world::Object proto) {
  if (!proto.has_kinematic_object_component()) {
    return absl::InternalError("Missing kinematic_object_component");
  }

  INTRINSIC_ASSIGN_OR_RETURN(
      JointLimitsXd joint_system_limits,
      ToJointLimitsXd(
          proto.kinematic_object_component().joint_system_limits()));

  INTRINSIC_ASSIGN_OR_RETURN(
      JointLimitsXd joint_application_limits,
      ToJointLimitsXd(
          proto.kinematic_object_component().joint_application_limits()));

  CartesianLimits cartesian_limits = CartesianLimits::Unlimited();
  if (proto.kinematic_object_component().has_cartesian_limits()) {
    INTRINSIC_ASSIGN_OR_RETURN(
        cartesian_limits,
        intrinsic::icon::FromProto(
            proto.kinematic_object_component().cartesian_limits()));
  }

  std::optional<double> control_frequency_hz;
  if (proto.kinematic_object_component().has_control_frequency_hz()) {
    control_frequency_hz =
        proto.kinematic_object_component().control_frequency_hz();
  }

  INTRINSIC_ASSIGN_OR_RETURN(std::shared_ptr<const WorldObject::Data> data,
                             CreateWorldObjectData(std::move(proto)));

  return KinematicObject(std::make_shared<const Data>(
      std::move(*data), joint_system_limits, joint_application_limits,
      cartesian_limits, control_frequency_hz));
}

std::optional<KinematicObject> KinematicObject::FromTransformNode(
    const TransformNode& node) {
  std::shared_ptr<const Data> object_data =
      std::dynamic_pointer_cast<const Data>(node.data_);
  return object_data ? std::optional<KinematicObject>(
                           KinematicObject(std::move(object_data)))
                     : std::nullopt;
}

KinematicObject::KinematicObject(std::shared_ptr<const Data> data)
    : WorldObject(data) {}

eigenmath::VectorXd KinematicObject::JointPositions() const {
  return RepeatedDoubleToVectorXd(
      GetData().Proto().kinematic_object_component().joint_positions());
}

const JointLimitsXd& KinematicObject::JointSystemLimits() const {
  return GetData().JointSystemLimits();
}

const JointLimitsXd& KinematicObject::JointApplicationLimits() const {
  return GetData().JointApplicationLimits();
}

std::vector<ObjectWorldResourceId> KinematicObject::IsoFlangeFrameIds() const {
  std::vector<ObjectWorldResourceId> result;
  for (const intrinsic_proto::world::IdAndName& id_and_name :
       GetData().Proto().kinematic_object_component().iso_flange_frames()) {
    result.push_back(ObjectWorldResourceId(id_and_name.id()));
  }
  return result;
}

std::vector<FrameName> KinematicObject::IsoFlangeFrameNames() const {
  std::vector<FrameName> result;
  for (const intrinsic_proto::world::IdAndName& id_and_name :
       GetData().Proto().kinematic_object_component().iso_flange_frames()) {
    result.push_back(FrameName(id_and_name.name()));
  }
  return result;
}

std::vector<Frame> KinematicObject::IsoFlangeFrames() const {
  std::vector<ObjectWorldResourceId> flange_ids = IsoFlangeFrameIds();

  std::vector<Frame> result;
  result.reserve(flange_ids.size());
  for (const Frame& frame : GetData().Frames()) {
    if (absl::c_find(flange_ids, frame.Id()) != flange_ids.end()) {
      result.push_back(frame);
    }
  }
  return result;
}

absl::StatusOr<Frame> KinematicObject::GetSingleIsoFlangeFrame() const {
  std::vector<Frame> frames = IsoFlangeFrames();
  if (frames.empty()) {
    return absl::NotFoundError(
        absl::Substitute("Kinematic object \"$0\" does not have any flange "
                         "frame configured, but exactly one was expected.",
                         Name().value()));
  } else if (frames.size() > 1) {
    return absl::NotFoundError(absl::Substitute(
        "Kinematic object \"$0\" has more than one flange "
        "frame configured, but exactly one was expected. The available flange "
        "frames are: $1.",
        Name().value(),
        absl::StrJoin(frames, ", ", [](std::string* out, const Frame& frame) {
          out->append(frame.Name().value());
        })));
  }
  return frames.front();
}

absl::StatusOr<intrinsic::CartesianLimits> KinematicObject::GetCartesianLimits()
    const {
  return GetData().CartesianLimits();
}

absl::StatusOr<std::optional<double>> KinematicObject::GetControlFrequencyHz()
    const {
  return GetData().ControlFrequencyHz();
}

const KinematicObject::Data& KinematicObject::GetData() const {
  // This has to succeed because instances of KinematicObject are always only
  // created with an instance of KinematicObject::Data or a subclass thereof.
  return static_cast<const KinematicObject::Data&>(TransformNode::GetData());
}

KinematicObject::Data::Data(WorldObject::Data data,
                            JointLimitsXd joint_system_limits,
                            JointLimitsXd joint_application_limits,
                            intrinsic::CartesianLimits cartesian_limits,
                            std::optional<double> control_frequency_hz)
    : WorldObject::Data(std::move(data)),
      joint_system_limits_(std::move(joint_system_limits)),
      joint_application_limits_(std::move(joint_application_limits)),
      cartesian_limits_(std::move(cartesian_limits)),
      control_frequency_hz_(std::move(control_frequency_hz)) {}

const JointLimitsXd& KinematicObject::Data::JointSystemLimits() const {
  return joint_system_limits_;
}

const JointLimitsXd& KinematicObject::Data::JointApplicationLimits() const {
  return joint_application_limits_;
}

const intrinsic::CartesianLimits& KinematicObject::Data::CartesianLimits()
    const {
  return cartesian_limits_;
}

const std::optional<double>& KinematicObject::Data::ControlFrequencyHz() const {
  return control_frequency_hz_;
}

}  // namespace world
}  // namespace intrinsic
