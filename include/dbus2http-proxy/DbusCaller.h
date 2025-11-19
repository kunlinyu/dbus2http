//
// Created by yukunlin on 2025/11/18.
//
#pragma once

#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IProxy.h>
#include <sdbus-c++/Types.h>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace dbus2http {

class DbusCaller {
 private:
  std::unique_ptr<sdbus::IConnection> conn_;

 public:
  DbusCaller() { conn_ = sdbus::createSystemBusConnection(); }

  void Call(const std::string& service_name, const std::string& object_path,
            const std::string& interface_name, const std::string& method_name) {

  }
  sdbus::Variant callDynamicMethod(const nlohmann::json& input) {
    // 解析服务信息
    std::string serviceName = input["service"].get<std::string>();
    std::string objectPath = input["object"].get<std::string>();
    std::string interfaceName = input["interface"].get<std::string>();
    std::string methodName = input["method"].get<std::string>();
    auto proxy =
        sdbus::createProxy(*conn_, sdbus::ServiceName("org.freedesktop.DBus"),
                           sdbus::ObjectPath("/org/freedesktop/DBus"));
    // 解析输入参数签名和值
    std::vector<std::string> argSigs =
        input["arg_signatures"].get<std::vector<std::string>>();
    std::vector<nlohmann::json> argValues =
        input["arg_values"].get<std::vector<nlohmann::json>>();

    if (argSigs.size() != argValues.size()) {
      throw std::invalid_argument(
          "Argument signatures and values size mismatch");
    }

    // 创建连接和方法调用
    auto connection = sdbus::createSessionBusConnection();

    auto methodCall = proxy->createMethodCall(
        sdbus::InterfaceName(interfaceName), sdbus::MethodName(methodName));
    ;

    // 动态打包参数
    for (size_t i = 0; i < argSigs.size(); ++i) {
      // sdbus::Variant arg = convertJsonToVariant(argSigs[i], argValues[i]);
      int a = 1;
      std::string s = "2";
      std::tuple t = std::make_tuple(a, s);
      methodCall << t;  // 用Variant传递动态参数
    }

    // 发送调用并获取响应
    // auto reply = connection->callMethod(methodCall, 5000);

    // 解析返回值（假设返回值签名已知，此处为"b"）
    sdbus::Variant result;
    // reply >> result;
    return result;
  }

  void FillMethod(sdbus::MethodCall& method, const nlohmann::json& json, const std::vector<std::string>& arg_sigs) {

  }

  template <typename T>
  T JsonToSigType(const nlohmann::json& value, const std::string& sig) {
    if (sig == "s") return value.get<std::string>();
    if (sig == "i") return value.get<int32_t>();
    if (sig == "b") return value.get<bool>();
    if (sig == "d") return value.get<double>();
    throw std::invalid_argument("Unsupported signature: " + sig);
  }
};

}  // namespace dbus2http
