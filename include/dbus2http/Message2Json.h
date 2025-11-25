//
// Created by yukunlin on 2025/11/20.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <nlohmann/json.hpp>

#include "entity/method.h"

namespace dbus2http {

class Message2Json {
 public:
  static nlohmann::json ExtractMessage(sdbus::Message& method_reply,
                                       const Method& method_type);

  static nlohmann::json ExtractVariant(sdbus::Message& method_reply);

  static nlohmann::json ExtractMessage(sdbus::Message& method_reply,
                                       const std::string& sig);

 private:
  template <typename T>
  static T get_int(sdbus::Message& method_reply) {
    T result;
    method_reply >> result;
    return result;
  }
};
};  // namespace dbus2http
