// Copyright 2023 Intrinsic Innovation LLC

#include "intrinsic/world/robot_payload/robot_payload.h"

#include <cstdlib>
#include <ostream>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "intrinsic/eigenmath/types.h"
#include "intrinsic/kinematics/validate_link_parameters.h"
#include "intrinsic/math/pose3.h"
#include "intrinsic/math/proto_conversion.h"
#include "intrinsic/util/status/status_macros.h"
#include "intrinsic/world/proto/robot_payload.pb.h"

namespace intrinsic {

namespace {

constexpr double kMassEpsilon = 1e-9;

bool MassAlmostEqual(double x, double y) {
  return std::abs(x - y) < kMassEpsilon;
}

}  // namespace

RobotPayload::RobotPayload()
    : mass_kg_(0.0),
      tip_t_cog_(Pose3d::Identity()),
      inertia_in_cog_(eigenmath::Matrix3d::Zero()) {}

RobotPayload::RobotPayload(double mass, const Pose3d& tip_t_cog,
                           const eigenmath::Matrix3d& inertia)
    : mass_kg_(mass), tip_t_cog_(tip_t_cog), inertia_in_cog_(inertia) {}

absl::Status RobotPayload::SetMass(double mass_kg) {
  // Allow a zero mass for robots without payload.
  if (!MassAlmostEqual(mass_kg, 0.0)) {
    INTR_RETURN_IF_ERROR(intrinsic::kinematics::ValidateMass(mass_kg));
  }
  mass_kg_ = mass_kg;
  return absl::OkStatus();
}

absl::Status RobotPayload::SetTipTCog(const Pose3d& tip_t_cog) {
  tip_t_cog_ = tip_t_cog;
  return absl::OkStatus();
}

absl::Status RobotPayload::SetInertia(const eigenmath::Matrix3d& inertia) {
  // Allow a zero matrix to handle point masses.
  if (!inertia.isZero()) {
    INTR_RETURN_IF_ERROR(intrinsic::kinematics::ValidateInertia(inertia));
  }

  inertia_in_cog_ = inertia;
  return absl::OkStatus();
}

absl::StatusOr<RobotPayload> RobotPayload::Create(
    double mass, const Pose3d& tip_t_cog, const eigenmath::Matrix3d& inertia) {
  RobotPayload payload;
  INTR_RETURN_IF_ERROR(payload.SetMass(mass));
  INTR_RETURN_IF_ERROR(payload.SetTipTCog(tip_t_cog));
  INTR_RETURN_IF_ERROR(payload.SetInertia(inertia));

  return payload;
}

bool RobotPayload::operator==(const RobotPayload& other) const {
  return MassAlmostEqual(mass_kg_, other.mass_kg_) &&
         tip_t_cog_.isApprox(other.tip_t_cog_) &&
         inertia_in_cog_.isApprox(other.inertia_in_cog_);
}

std::ostream& operator<<(std::ostream& os, const RobotPayload& payload) {
  os << "Payload: mass: " << payload.mass()
     << " tip_t_cog: " << payload.tip_t_cog()
     << " inertia: " << payload.inertia();
  return os;
}

absl::StatusOr<RobotPayload> FromProto(
    const intrinsic_proto::world::RobotPayload& proto) {
  Pose3d tip_t_cog;
  if (proto.has_tip_t_cog()) {
    INTR_ASSIGN_OR_RETURN(tip_t_cog,
                          intrinsic_proto::FromProto(proto.tip_t_cog()));
  }

  eigenmath::Matrix3d inertia = eigenmath::Matrix3d::Zero();
  if (proto.has_inertia()) {
    if (proto.inertia().rows() != 3 || proto.inertia().cols() != 3) {
      return absl::InvalidArgumentError("Inertia must be 3x3 matrix.");
    }
    INTR_ASSIGN_OR_RETURN(inertia, intrinsic_proto::FromProto(proto.inertia()));
  }

  return RobotPayload::Create(proto.mass_kg(), tip_t_cog, inertia);
}

intrinsic_proto::world::RobotPayload ToProto(const RobotPayload& payload) {
  intrinsic_proto::world::RobotPayload proto;
  proto.set_mass_kg(payload.mass());
  *proto.mutable_tip_t_cog() = ToProto(payload.tip_t_cog());
  *proto.mutable_inertia() = ToProto(payload.inertia());
  return proto;
}

}  // namespace intrinsic
