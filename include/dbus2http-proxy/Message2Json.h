//
// Created by yukunlin on 2025/11/20.
//

#pragma once

#include <nlohmann/json.hpp>
#include <sdbus-c++/sdbus-c++.h>

namespace dbus2http {

class Message2Json {
public:
  static nlohmann::json ExtractMethod(sdbus::MethodReply& method_reply) {
    nlohmann::json result;
    // method_reply >> result;
    int i;
        method_reply >> i;
    if (method_reply) method_reply << i;
    method_reply.clearFlags();
    return result;
  }
};

}  // namespace dbus2http

