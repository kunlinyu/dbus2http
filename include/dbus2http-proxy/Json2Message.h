//
// Created by yukunlin on 2025/11/20.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "entity/method.h"

namespace dbus2http {

class Json2Message {
 public:
  static void FillMethod(sdbus::MethodCall& method_call,
                         const Method& method_type,
                         const nlohmann::json& json);

  static void FillDictToMethod(sdbus::MethodCall& method_call,
                            const std::string& key, const nlohmann::json& json,
                            const std::string& sig);

  static void FillMethodSig(sdbus::MethodCall& method_call,
                            const nlohmann::json& json,
                            const std::string& sig);

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
  static void AppendIntegerFromJson(sdbus::MethodCall& method_call,
                                     const nlohmann::json& json) {
    if (json.is_number_integer()) {
      std::cout << "typed append " << std::string(typeid(T).name()) << " "
                << json.dump() << std::endl;
      method_call << json.get<T>();
    } else
      throw std::invalid_argument(
          "Expected " + std::string(typeid(T).name()) +
          " type but we get :" + std::string(json.type_name()));
  }

};


}  // namespace dbus2http
