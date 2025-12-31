//
// Created by yukunlin on 2025/11/20.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <nlohmann/json.hpp>

#include "Config.h"
#include "entity/argument.h"

namespace dbus2http {

class Message2Json {
  Config config_;

 public:
  Message2Json(Config config) : config_(config) {}
  nlohmann::json ExtractMessage(sdbus::Message& message,
                                const std::vector<Argument>& args);

  nlohmann::json ExtractVariant(sdbus::Message& message);

  nlohmann::json ExtractMessage(sdbus::Message& message,
                                const std::string& sig);

  static nlohmann::json WrapHeader(sdbus::Message& message,
                                   const nlohmann::json& j);

  static bool contains_binaries(const nlohmann::json& j) {
    if (j.is_binary()) return true;
    if (j.is_array())
      for (const auto& elem : j)
        if (contains_binaries(elem)) return true;
    if (j.is_object())
      for (auto it = j.begin(); it != j.end(); ++it)
        if (contains_binaries(it.value())) return true;
    return false;
  }

 private:
  static nlohmann::json json_null(const char* value) {
    if (value == nullptr) return nullptr;
    return std::string(value);
  }

  template <typename T>
  static T get_int(sdbus::Message& message) {
    T result;
    message >> result;
    return result;
  }
};
};  // namespace dbus2http
