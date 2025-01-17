// Copyright 2023 Intrinsic Innovation LLC

#include "intrinsic/platform/pubsub/pubsub.h"

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/message.h"
#include "intrinsic/platform/pubsub/publisher.h"
#include "intrinsic/platform/pubsub/subscription.h"
#include "pybind11_abseil/no_throw_status.h"
#include "pybind11_abseil/status_casters.h"
#include "pybind11_protobuf/native_proto_caster.h"

namespace intrinsic {
namespace pubsub {

namespace {

absl::StatusOr<Subscription> PySubscriptionWithConfig(
    PubSub* self, absl::string_view topic, const TopicConfig& config,
    const google::protobuf::Message& exemplar, pybind11::object msg_callback,
    pybind11::object err_callback) {
  // The callback passed to the adapter must be able to be copied in a
  // separate thread without copying the msg_callback.
  // This allows the message callback to capture variables which
  // are not possible (or safe) to copy in a separate thread. This is the
  // case when the callback captures a python function, since those cannot
  // be copied without holding the GIL, and the adapter thread executing the
  // callback does not know to acquire the GIL. Using a shared pointer to
  // own the adapter callback satisfies these requirements.

  SubscriptionOkCallback<google::protobuf::Message> message_callback = {};
  SubscriptionErrorCallback error_callback = {};

  if (msg_callback && !msg_callback.is_none()) {
    // Shared pointer to a function that wraps the python callback.
    auto locked_msg_callback =
        std::make_shared<std::function<void(const google::protobuf::Message&)>>(
            [msg_callback](const google::protobuf::Message& msg) {
              // Create a copy to hand over ownership to the Python callback.
              // If we just pass a pointer, it's easy for the Python callback to
              // hold onto the reference to it for too long (e.g. by copying it
              // out of the callback).
              auto copy = absl::WrapUnique(msg.New());
              copy->CopyFrom(msg);
              pybind11::gil_scoped_acquire gil;
              msg_callback(*copy);
            });
    message_callback = [locked_msg_callback = std::move(locked_msg_callback)](
                           const google::protobuf::Message& message) {
      (*locked_msg_callback)(message);
    };
  }

  if (err_callback && !err_callback.is_none()) {
    auto locked_error_callback =
        std::make_shared<std::function<void(absl::string_view, absl::Status)>>(
            [err_callback](absl::string_view packet, absl::Status error) {
              pybind11::gil_scoped_acquire gil;
              err_callback(packet, pybind11::google::DoNotThrowStatus(error));
            });
    error_callback = [locked_error_callback = std::move(locked_error_callback)](
                         absl::string_view packet, absl::Status error) {
      (*locked_error_callback)(packet, error);
    };
  }

  return self->CreateSubscription(topic, config, exemplar,
                                  std::move(message_callback),
                                  std::move(error_callback));
}

absl::StatusOr<Subscription> PySubscription(
    PubSub* self, absl::string_view topic,
    const google::protobuf::Message& exemplar, pybind11::object msg_callback,
    pybind11::object err_callback) {
  return PySubscriptionWithConfig(self, topic, TopicConfig{}, exemplar,
                                  std::move(msg_callback),
                                  std::move(err_callback));
}

}  // namespace

PYBIND11_MODULE(pubsub, m) {
  pybind11::google::ImportStatusModule();
  pybind11_protobuf::ImportNativeProtoCasters();

  pybind11::enum_<TopicConfig::TopicQoS>(m, "TopicQoS")
      .value("HighReliability", TopicConfig::TopicQoS::HighReliability)
      .value("Sensor", TopicConfig::TopicQoS::Sensor)
      .export_values();

  pybind11::class_<TopicConfig>(m, "TopicConfig")
      .def(pybind11::init<>())
      .def_readwrite("topic_qos", &TopicConfig::topic_qos);

  pybind11::class_<PubSub>(m, "PubSub")
      .def(pybind11::init<>())
      .def(pybind11::init<std::string_view>(),
           pybind11::arg("participant_name"))
      .def(pybind11::init<std::string_view, std::string_view>(),
           pybind11::arg("participant_name"), pybind11::arg("config"))
      // Cast required for overloaded methods:
      // https://pybind11.readthedocs.io/en/stable/classes.html#overloaded-methods
      .def("CreatePublisher", &PubSub::CreatePublisher, pybind11::arg("topic"),
           pybind11::arg("config") = TopicConfig{})
      .def("CreateSubscription", &PySubscriptionWithConfig,
           pybind11::arg("topic"), pybind11::arg("config"),
           pybind11::arg("exemplar"), pybind11::arg("msg_callback") = nullptr,
           pybind11::arg("error_callback") = nullptr)
      .def("CreateSubscription", &PySubscription, pybind11::arg("topic"),
           pybind11::arg("exemplar"), pybind11::arg("msg_callback") = nullptr,
           pybind11::arg("error_callback") = nullptr);

  pybind11::class_<Publisher>(m, "Publisher")
      .def("Publish",
           static_cast<absl::Status (Publisher::*)(
               const google::protobuf::Message&) const>(&Publisher::Publish),
           pybind11::arg("message"))
      .def("TopicName", &Publisher::TopicName);

  pybind11::class_<Subscription>(m, "Subscription")
      .def("TopicName", &Subscription::TopicName);
}

}  // namespace pubsub
}  // namespace intrinsic
