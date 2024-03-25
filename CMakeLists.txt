cmake_minimum_required(VERSION 3.20)
project(intrinsic_sdks LANGUAGES CXX)

# Default to C++20
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()


find_package(ament_cmake REQUIRED)
find_package(grpc_vendor REQUIRED)
find_package(absl REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(re2 REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googleapis
  GIT_REPOSITORY https://github.com/googleapis/googleapis
  GIT_TAG        master
)
FetchContent_MakeAvailable(googleapis)

set(intrinsic_proto_SRCS
  intrinsic/assets/proto/asset_tag.proto
  intrinsic/assets/proto/asset_type.proto
  intrinsic/assets/proto/documentation.proto
  intrinsic/assets/proto/id.proto
  intrinsic/assets/proto/metadata.proto
  intrinsic/assets/proto/release_tag.proto
  intrinsic/assets/proto/vendor.proto
  intrinsic/assets/proto/view.proto
  intrinsic/config/proto/ppr_refs.proto
  intrinsic/executive/proto/annotations.proto
  intrinsic/executive/proto/any_list.proto
  intrinsic/executive/proto/any_with_assignments.proto
  intrinsic/executive/proto/behavior_call.proto
  intrinsic/executive/proto/behavior_tree.proto
  intrinsic/executive/proto/domain.proto
  intrinsic/executive/proto/executive_execution_mode.proto
  intrinsic/executive/proto/executive_service.proto
  intrinsic/executive/proto/executive_state.proto
  intrinsic/executive/proto/run_metadata.proto
  intrinsic/executive/proto/run_response.proto
  intrinsic/executive/proto/test_message.proto
  intrinsic/executive/proto/world_query.proto
  intrinsic/frontend/cloud/api/clusterdeletion_api.proto
  intrinsic/frontend/cloud/api/clusterdiscovery_api.proto
  intrinsic/frontend/cloud/api/orgdiscovery_api.proto
  intrinsic/frontend/cloud/api/projectdiscovery_api.proto
  intrinsic/frontend/cloud/api/solutiondiscovery_api.proto
  intrinsic/geometry/proto/axis_aligned_bounding_box.proto
  intrinsic/geometry/proto/geometry.proto
  intrinsic/geometry/proto/lazy_exact_geometry.proto
  intrinsic/geometry/proto/mesh_octree_wrapping.proto
  intrinsic/geometry/proto/octree.proto
  intrinsic/geometry/proto/oriented_bounding_box.proto
  intrinsic/geometry/proto/primitives.proto
  intrinsic/geometry/proto/renderable.proto
  intrinsic/geometry/proto/shape_data.proto
  intrinsic/geometry/proto/triangle_mesh.proto
  intrinsic/geometry/service/geometry_service.proto
  intrinsic/geometry/service/geometry_service_types.proto
  intrinsic/geometry/service/geometry_storage_refs.proto
  intrinsic/hardware/gpio/gpio_service.proto
  intrinsic/hardware/gpio/signal.proto
  intrinsic/hardware/opcua_equipment/opcua_equipment_service.proto
  intrinsic/hardware/proto/settings.proto
  intrinsic/icon/actions/adio.proto
  intrinsic/icon/actions/blended_move_action.proto
  intrinsic/icon/actions/cartesian_jogging.proto
  intrinsic/icon/actions/point_to_point_move.proto
  intrinsic/icon/actions/simple_gripper.proto
  intrinsic/icon/actions/tare_force_torque_sensor.proto
  intrinsic/icon/actions/trajectory_tracking_action.proto
  intrinsic/icon/actions/wait_for_settling_action.proto
  intrinsic/icon/equipment/force_control_settings.proto
  intrinsic/icon/equipment/icon_equipment.proto
  intrinsic/icon/examples/joint_move_positions.proto
  intrinsic/icon/proto/blended_cartesian_move.proto
  intrinsic/icon/proto/cart_space.proto
  intrinsic/icon/proto/generic_part_config.proto
  intrinsic/icon/proto/ik_options.proto
  intrinsic/icon/proto/io_block.proto
  intrinsic/icon/proto/joint_position_pid_torque_controller_config.proto
  intrinsic/icon/proto/joint_space.proto
  intrinsic/icon/proto/linear_joint_acceleration_filter_config.proto
  intrinsic/icon/proto/matrix.proto
  intrinsic/icon/proto/part_status.proto
  intrinsic/icon/proto/safety_status.proto
  intrinsic/icon/proto/service.proto
  intrinsic/icon/proto/streaming_output.proto
  intrinsic/icon/proto/types.proto
  intrinsic/kinematics/ik/constrained/constrained_ik.proto
  intrinsic/kinematics/proto/kinematics.proto
  intrinsic/kinematics/proto/skeleton.proto
  intrinsic/kinematics/types/dynamic_limits_check_mode.proto
  intrinsic/kinematics/types/joint_limits.proto
  intrinsic/kubernetes/workcell_spec/proto/image.proto
  intrinsic/kubernetes/workcell_spec/proto/installer.proto
  intrinsic/logging/errors/proto/error_report.proto
  intrinsic/logging/proto/blob.proto
  intrinsic/logging/proto/context.proto
  intrinsic/logging/proto/flowstate_event.proto
  intrinsic/logging/proto/log_dispatcher_service.proto
  intrinsic/logging/proto/logger_service.proto
  intrinsic/logging/proto/log_item.proto
  intrinsic/logging/proto/pubsub_listener_service.proto
  intrinsic/math/proto/affine.proto
  intrinsic/math/proto/array.proto
  intrinsic/math/proto/header.proto
  intrinsic/math/proto/matrix.proto
  intrinsic/math/proto/point.proto
  intrinsic/math/proto/pose.proto
  intrinsic/math/proto/quaternion.proto
  intrinsic/math/proto/tf_message.proto
  intrinsic/math/proto/transform.proto
  intrinsic/math/proto/transform_stamped.proto
  intrinsic/math/proto/vector2.proto
  intrinsic/math/proto/vector3.proto
  intrinsic/motion_planning/proto/motion_planner_config.proto
  intrinsic/motion_planning/proto/motion_planner_service.proto
  intrinsic/motion_planning/proto/motion_planning_error.proto
  intrinsic/motion_planning/proto/motion_specification.proto
  intrinsic/motion_planning/proto/motion_target.proto
  intrinsic/motion_planning/proto/robot_specification.proto
  intrinsic/motion_planning/trajectory_planning/blended_joint_move.proto
  intrinsic/perception/proto/camera_config.proto
  intrinsic/perception/proto/camera_drivers.proto
  intrinsic/perception/proto/camera_identifier.proto
  intrinsic/perception/proto/camera_params.proto
  intrinsic/perception/proto/camera_settings.proto
  intrinsic/perception/proto/capture_result.proto
  intrinsic/perception/proto/charuco_pattern.proto
  intrinsic/perception/proto/dimensions.proto
  intrinsic/perception/proto/distortion_params.proto
  intrinsic/perception/proto/frame_post_processing.proto
  intrinsic/perception/proto/frame.proto
  intrinsic/perception/proto/hand_eye_calibration.proto
  intrinsic/perception/proto/image_buffer.proto
  intrinsic/perception/proto/image_processing.proto
  intrinsic/perception/proto/intrinsic_calibration.proto
  intrinsic/perception/proto/intrinsic_params.proto
  intrinsic/perception/proto/pattern_detection_config.proto
  intrinsic/perception/proto/pattern_detection_result.proto
  intrinsic/perception/proto/point_cloud.proto
  intrinsic/perception/proto/pose_estimation_result.proto
  intrinsic/perception/proto/pose_estimator_id.proto
  intrinsic/perception/proto/sensor_config.proto
  intrinsic/perception/proto/sensor_image.proto
  intrinsic/perception/proto/vector.proto
  intrinsic/perception/service/proto/camera_server.proto
  intrinsic/platform/common/proto/test.proto
  intrinsic/platform/file_upload/file_upload_service.proto
  intrinsic/platform/pubsub/adapters/pubsub.proto
  intrinsic/resources/proto/resource_handle.proto
  intrinsic/resources/proto/resource_health.proto
  intrinsic/resources/proto/resource_operational_status.proto
  intrinsic/resources/proto/resource_registry.proto
  intrinsic/simulation/service/proto/simulation_service.proto
  intrinsic/skills/apps/calibration/collect_calibration_data.proto
  intrinsic/skills/apps/calibration/optimize_robot_and_camera_poses.proto
  intrinsic/skills/apps/calibration/sample_calibration_poses.proto
  intrinsic/skills/internal/proto/behavior_tree_registry_internal.proto
  intrinsic/skills/internal/proto/skill_registry_internal.proto
  intrinsic/skills/proto/behavior_tree_registry.proto
  intrinsic/skills/proto/equipment.proto
  intrinsic/skills/proto/error.proto
  intrinsic/skills/proto/footprint.proto
  intrinsic/skills/proto/motion_targets.proto
  intrinsic/skills/proto/prediction.proto
  intrinsic/skills/proto/skill_manifest.proto
  intrinsic/skills/proto/skill_parameter_metadata.proto
  intrinsic/skills/proto/skill_registry_config.proto
  intrinsic/skills/proto/skill_registry.proto
  intrinsic/skills/proto/skill_service_config.proto
  intrinsic/skills/proto/skill_service.proto
  intrinsic/skills/proto/skills.proto
  intrinsic/skills/testing/no_op_skill.proto
  intrinsic/solutions/testing/test_skill_params.proto
  intrinsic/storage/artifacts/proto/artifact.proto
  intrinsic/util/grpc/testing/ping.proto
  intrinsic/util/proto/build_defs/testing/test_message_dep.proto
  intrinsic/util/proto/build_defs/testing/test_message.proto
  #intrinsic/util/proto/testing/config_node_to_proto_test.proto
  intrinsic/util/proto/testing/descriptors_test_left_wrapper.proto
  intrinsic/util/proto/testing/descriptors_test.proto
  intrinsic/util/proto/testing/descriptors_test_right_wrapper.proto
  intrinsic/util/proto/testing/diamond_a.proto
  intrinsic/util/proto/testing/diamond_b.proto
  intrinsic/util/proto/testing/diamond_c.proto
  intrinsic/util/proto/testing/diamond_d.proto
  intrinsic/util/proto/testing/embedded.proto
  intrinsic/util/proto/testing/get_text_proto_test.proto
  intrinsic/util/proto/testing/recursive.proto
  intrinsic/util/status/status.proto
  intrinsic/world/proto/collision_action.proto
  intrinsic/world/proto/collision_settings.proto
  intrinsic/world/proto/entity_search.proto
  intrinsic/world/proto/geometric_constraints.proto
  intrinsic/world/proto/geometry_component.proto
  intrinsic/world/proto/gripper_component.proto
  intrinsic/world/proto/kinematics_component.proto
  intrinsic/world/proto/object_world_refs.proto
  intrinsic/world/proto/object_world_service.proto
  intrinsic/world/proto/object_world_updates.proto
  intrinsic/world/proto/physics_component.proto
  intrinsic/world/proto/robot_payload.proto
  intrinsic/world/proto/sensor_component.proto
  intrinsic/world/proto/simulation_component.proto
  intrinsic/world/proto/spawner_component.proto
  ${googleapis_SOURCE_DIR}/google/api/annotations.proto
  ${googleapis_SOURCE_DIR}/google/api/client.proto
  ${googleapis_SOURCE_DIR}/google/api/http.proto
  ${googleapis_SOURCE_DIR}/google/api/launch_stage.proto
  ${googleapis_SOURCE_DIR}/google/longrunning/operations.proto
  ${googleapis_SOURCE_DIR}/google/rpc/code.proto
  ${googleapis_SOURCE_DIR}/google/rpc/status.proto
  src/proto/grpc/health/v1/health.proto
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protos_gen)
add_library(protos STATIC ${intrinsic_proto_SRCS})
protobuf_generate(
  TARGET protos
  LANGUAGE cpp
  IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${googleapis_SOURCE_DIR}
  PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/protos_gen
)
target_include_directories(protos
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/protos_gen>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}/protos_gen>"
)
target_link_libraries(protos PUBLIC protobuf::libprotobuf)

add_library(services
  STATIC
  intrinsic/logging/proto/logger_service.proto
  intrinsic/motion_planning/proto/motion_planner_service.proto
  intrinsic/perception/service/proto/camera_server.proto
  intrinsic/skills/proto/behavior_tree_registry.proto
  intrinsic/skills/proto/skill_registry.proto
  intrinsic/skills/proto/skill_service.proto
  intrinsic/skills/internal/proto/behavior_tree_registry_internal.proto
  intrinsic/skills/internal/proto/skill_registry_internal.proto
  intrinsic/world/proto/object_world_service.proto
  src/proto/grpc/health/v1/health.proto
)

protobuf_generate(
  TARGET services
  LANGUAGE grpc
  PLUGIN protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
  IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${googleapis_SOURCE_DIR}
  PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/protos_gen
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
)
target_include_directories(services
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/protos_gen>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}/protos_gen>"
)
target_link_libraries(services PUBLIC protos protobuf::libprotobuf)

add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/intrinsic_proto.desc
      COMMAND protobuf::protoc
      ARGS
        -I${CMAKE_CURRENT_SOURCE_DIR}
        -I${googleapis_SOURCE_DIR}
        --include_imports
        --descriptor_set_out=${CMAKE_CURRENT_BINARY_DIR}/intrinsic_proto.desc
        ${intrinsic_proto_SRCS}
      DEPENDS ${protobuf_PROTOC_EXE} ${intrinsic_proto_SRCS}
      COMMENT "Generating descriptor set"
      VERBATIM )
add_custom_target(intrinsic_proto_desc ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/intrinsic_proto.desc)

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/protos_gen"
        DESTINATION "include/${PROJECT_NAME}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/intrinsic_proto.desc"
        DESTINATION "share/${PROJECT_NAME}"
)

add_executable(skill_service_config_main
  intrinsic/util/proto/source_code_info_view.cc
  intrinsic/util/status/status_builder.cc
  intrinsic/platform/common/buffers/realtime_write_queue.cc
  intrinsic/platform/common/buffers/internal/event_fd.cc
  intrinsic/skills/internal/skill_service_config_main.cc
  intrinsic/skills/internal/skill_proto_utils.cc
  intrinsic/icon/interprocess/binary_futex.cc
  intrinsic/icon/release/portable/init_xfa_absl.cc
  intrinsic/icon/utils/log.cc
  intrinsic/icon/utils/log_internal.cc
  intrinsic/icon/utils/log_sink.cc
  intrinsic/icon/utils/realtime_log_sink.cc
  intrinsic/icon/utils/realtime_guard.cc
  intrinsic/icon/utils/realtime_status.cc
)
target_link_libraries(skill_service_config_main
  PUBLIC
  absl::log absl::status absl::statusor absl::flags absl::flags_usage absl::flags_parse
  re2::re2
  protos
)
target_include_directories(skill_service_config_main PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")


add_library(${PROJECT_NAME}
  STATIC
  intrinsic/assets/id_utils.cc

  intrinsic/icon/interprocess/binary_futex.cc
  intrinsic/icon/proto/cart_space_conversion.cc
  intrinsic/icon/proto/eigen_conversion.cc
  intrinsic/icon/release/grpc_time_support.cc
  intrinsic/icon/release/portable/init_xfa_absl.cc
  intrinsic/icon/utils/log.cc
  intrinsic/icon/utils/log_internal.cc
  intrinsic/icon/utils/log_sink.cc
  intrinsic/icon/utils/realtime_guard.cc
  intrinsic/icon/utils/realtime_log_sink.cc
  intrinsic/icon/utils/realtime_status.cc

  intrinsic/kinematics/types/cartesian_limits.cc
  intrinsic/kinematics/types/dynamic_limits_check_mode.cc
  intrinsic/kinematics/types/joint_limits.cc
  intrinsic/kinematics/types/joint_limits_xd.cc

  intrinsic/logging/data_logger_client.cc
  intrinsic/logging/structured_logging_client.cc

  intrinsic/math/proto_conversion.cc
  intrinsic/math/transform_utils.cc
  intrinsic/math/twist.cc

  intrinsic/motion_planning/conversions.cc
  intrinsic/motion_planning/motion_planner_client.cc

  intrinsic/platform/common/buffers/realtime_write_queue.cc
  intrinsic/platform/common/buffers/internal/event_fd.cc

  intrinsic/skills/cc/equipment_pack.cc
  intrinsic/skills/cc/skill_canceller.cc
  intrinsic/skills/cc/skill_interface_utils.cc
  intrinsic/skills/cc/skill_utils.cc

  intrinsic/skills/internal/default_parameters.cc
  intrinsic/skills/internal/equipment_utilities.cc
  intrinsic/skills/internal/error_utils.cc
  intrinsic/skills/internal/get_footprint_context_impl.cc
  intrinsic/skills/internal/preview_context_impl.cc
  intrinsic/skills/internal/runtime_data.cc
  intrinsic/skills/internal/single_skill_factory.cc
  intrinsic/skills/internal/skill_init.cc
  intrinsic/skills/internal/skill_proto_utils.cc
  intrinsic/skills/internal/skill_registry_client.cc
  intrinsic/skills/internal/skill_service_impl.cc

  intrinsic/util/grpc/channel.cc
  intrinsic/util/grpc/channel_interface.cc
  intrinsic/util/grpc/connection_params.cc
  intrinsic/util/grpc/grpc.cc

  intrinsic/util/status/annotate.cc
  intrinsic/util/status/ret_check.cc
  intrinsic/util/status/ret_check_grpc.cc
  intrinsic/util/status/status_builder.cc
  intrinsic/util/status/status_builder_grpc.cc
  intrinsic/util/status/status_conversion_grpc.cc
  intrinsic/util/status/status_conversion_proto.cc
  intrinsic/util/status/status_conversion_rpc.cc

  intrinsic/util/proto/descriptors.cc
  intrinsic/util/proto/get_text_proto.cc
  intrinsic/util/proto/merge.cc
  intrinsic/util/proto/source_code_info_view.cc

  intrinsic/util/thread/lockstep.cc
  intrinsic/util/thread/thread.cc
  intrinsic/util/thread/util.cc

  intrinsic/util/page_fault_info.cc
  intrinsic/util/proto_time.cc

  intrinsic/world/objects/frame.cc
  intrinsic/world/objects/kinematic_object.cc
  intrinsic/world/objects/object_entity_filter.cc
  intrinsic/world/objects/object_world_client.cc
  intrinsic/world/objects/object_world_client_utils.cc
  intrinsic/world/objects/object_world_ids.cc
  intrinsic/world/objects/transform_node.cc
  intrinsic/world/objects/world_object.cc
)


target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Eigen3::Eigen
    absl::flags_parse absl::flags_usage
    absl::log
    absl::time gRPC::grpc++
    absl::log_internal_check_op
    protos services
)

install(
  TARGETS ${PROJECT_NAME} protos services skill_service_config_main EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(PROGRAMS "${CMAKE_SOURCE_DIR}/cmake/generate_manifest.sh" # source directory
        DESTINATION "share/${PROJECT_NAME}/cmake" # target directory
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/intrinsic" # source directory
        DESTINATION "include/${PROJECT_NAME}" # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/intrinsic" # source directory
        DESTINATION "share/${PROJECT_NAME}/protos" # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.proto" # select header files
)

ament_export_targets(${PROJECT_NAME})
ament_export_dependencies(grpc_vendor)
ament_export_dependencies(absl)
ament_export_dependencies(Protobuf)
ament_export_dependencies(gRPC)
ament_export_dependencies(Eigen3)

ament_package(CONFIG_EXTRAS intrinsic_sdks-extras.cmake.in)
