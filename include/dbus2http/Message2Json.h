//
// Created by yukunlin on 2025/11/20.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <nlohmann/json.hpp>

#include "entity/argument.h"

namespace dbus2http {

class Message2Json {
 public:
  static nlohmann::json ExtractMessage(
      sdbus::Message& message, const std::vector<Argument>& method_type);

  static nlohmann::json ExtractVariant(sdbus::Message& message);

  static nlohmann::json ExtractMessage(sdbus::Message& message,
                                       const std::string& sig);

  static nlohmann::json WrapHeader(sdbus::Message& message, const nlohmann::json& j);

 private:
  static nlohmann::json json_null(const char* value) {
    if (value == nullptr) return nullptr;
    return std::string(value);
  }

  template <typename T>
  static T get_int(sdbus::Message& method_reply) {
    T result;
    method_reply >> result;
    return result;
  }
};
};  // namespace dbus2http
