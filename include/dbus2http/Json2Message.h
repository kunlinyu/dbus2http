//
// Created by yukunlin on 2025/11/20.
//

#pragma once

#include <plog/Log.h>
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "entity/method.h"

namespace dbus2http {

class Json2Message {
 public:
  static void FillMessage(sdbus::Message& message, const Method& method_type,
                          const nlohmann::json& json);

  static void FillMessage(sdbus::Message& message, const std::string& sig,
                          const nlohmann::json& json);

  static void FillVariant(sdbus::Message& message, const nlohmann::json& json);

 private:
  template <typename T>
  static T extract_integer(const std::string& key, char sig) {
    long long integer = std::stoll(key);
    if (integer >= std::numeric_limits<T>::min() &&
        integer <= std::numeric_limits<T>::max())
      return static_cast<T>(integer);
    throw std::invalid_argument(
        key + "Exceeds the valid range of target type " + sig);
  }

  template <typename T>
  static void AppendIntegerFromJson(sdbus::Message& message,
                                    const nlohmann::json& json) {
    if (json.is_number_integer()) {
      PLOGD << "typed append " << std::string(typeid(T).name()) << " "
            << json.dump();
      message << json.get<T>();
    } else if (json.is_string()) {
      long long integer = std::stoll(json.get<std::string>());
      if (integer >= std::numeric_limits<T>::min() &&
          integer <= std::numeric_limits<T>::max())
        message << static_cast<T>(integer);
      else
        throw std::invalid_argument(json.get<std::string>() +
                                    " Exceeds the valid range of target type");
    } else {
      std::string error_msg =
          "Expected " + std::string(typeid(T).name()) +
          " type but we get :" + std::string(json.type_name());
      PLOGE << error_msg;
      throw std::invalid_argument(error_msg);
    }
  }
};

}  // namespace dbus2http
