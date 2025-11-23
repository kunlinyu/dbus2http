//
// Created by yukunlin on 2025/11/18.
//
#pragma once

#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IProxy.h>
#include <sdbus-c++/Types.h>

#include <nlohmann/json.hpp>
#include <string>

#include "Json2Message.h"
#include "Message2Json.h"
#include "entity/InterfaceContext.h"

namespace dbus2http {

class DbusCaller {
  std::unique_ptr<sdbus::IConnection> conn_;
  const InterfaceContext& context_;

 public:
  explicit DbusCaller(const InterfaceContext& context) : context_(context) {
    conn_ = sdbus::createSessionBusConnection();
  }

  [[nodiscard]] nlohmann::json Call(const std::string& service_name, const std::string& object_path,
            const std::string& interface_name, const std::string& method_name,
            const nlohmann::json& request) const {
    auto proxy = sdbus::createProxy(sdbus::ServiceName(service_name),
                                    sdbus::ObjectPath(object_path));
    auto method = proxy->createMethodCall(sdbus::InterfaceName(interface_name),
                                          sdbus::MethodName(method_name));
    Method method_type = context_.GetMethod(interface_name, method_name);
    std::cout << "=====fill method call====" << std::endl;
    Json2Message::FillMethod(
        method, context_.GetMethod(interface_name, method_name), request);
    std::cout << "=====call====" << std::endl;
    auto reply = proxy->callMethod(method);
    std::cout << "=====extract method reply====" << std::endl;
    return Message2Json::ExtractMethod(reply, method_type);
  }
};

}  // namespace dbus2http
