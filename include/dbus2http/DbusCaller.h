//
// Created by yukunlin on 2025/11/18.
//
#pragma once

#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IProxy.h>
#include <sdbus-c++/Types.h>

#include <nlohmann/json.hpp>
#include <plog/Log.h>

#include <string>

#include "DbusUtils.h"
#include "Json2Message.h"
#include "Message2Json.h"
#include "entity/InterfaceContext.h"

namespace dbus2http {

class DbusCaller {
  std::unique_ptr<sdbus::IConnection> conn_;
  const InterfaceContext& context_;

 public:
  explicit DbusCaller(const InterfaceContext& context, bool system) : context_(context) {
    conn_ = DbusUtils::createConnection(system);
  }

  const InterfaceContext& context() const { return context_; }

  [[nodiscard]] nlohmann::json Call(const std::string& service_name, const std::string& object_path,
            const std::string& interface_name, const std::string& method_name,
            const nlohmann::json& request) const {
    auto proxy = sdbus::createProxy(sdbus::ServiceName(service_name),
                                    sdbus::ObjectPath(object_path));
    auto method = proxy->createMethodCall(sdbus::InterfaceName(interface_name),
                                          sdbus::MethodName(method_name));
    Method method_type = context_.GetMethod(interface_name, method_name);
    PLOGD << "=====fill method call====";
    Json2Message::FillMessage(
        method, context_.GetMethod(interface_name, method_name), request);
    PLOGD << "=====call====";
    auto reply = proxy->callMethod(method);
    PLOGD << "=====extract method reply====";
    return Message2Json::ExtractMessage(reply, method_type);
  }
};

}  // namespace dbus2http
