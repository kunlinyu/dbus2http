//
// Created by yukunlin on 2025/11/20.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <nlohmann/json.hpp>

namespace dbus2http {

class Message2Json {
 public:
  static nlohmann::json ExtractMethod(sdbus::MethodReply& method_reply, const Method& method_type) {
    nlohmann::json result;
    for (const auto& arg : method_type.args) {
      if (arg.direction != "out") continue;
      result[arg.name] = ExtractMethod2(method_reply, method_type, arg.type);
    }
    return result;

    // nlohmann::json result;
    // std::map<int32_t, bool> params;
    // method_reply >> params;
    // // method_reply.clearFlags();
    // std::map<std::string, bool> params_2;
    // for (const auto& [key, value] : params) {
    //   params_2[std::to_string(key)] = value;
    // }
    // std::cout << "convert to json" << std::endl;
    // nlohmann::json j = params_2;
    // std::cout << "done: " << j.dump(2) << std::endl;
    // return j;
  }

  static nlohmann::json ExtractMethod2(sdbus::MethodReply& method_reply, const Method& method_type, const std::string& sig) {

  }
};

}  // namespace dbus2http
