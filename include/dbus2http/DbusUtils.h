//
// Created by yukunlin on 2025/11/24.
//

#pragma once
#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/Types.h>

namespace dbus2http {
class DbusUtils {
 public:
  static std::unique_ptr<sdbus::IConnection> createConnection(bool system_bus) {
    try {
      if (system_bus) return sdbus::createSystemBusConnection();
      return sdbus::createSessionBusConnection();
    } catch (const std::exception& e) {
      PLOGE << "create connection failed: " << e.what();
      throw;
    }
  }
  static std::unique_ptr<sdbus::IConnection> createConnection(
      const std::string& service_name, bool system_bus) {
    try {
      if (system_bus)
        return sdbus::createSystemBusConnection(sdbus::ServiceName(service_name));
      return sdbus::createSessionBusConnection(sdbus::ServiceName(service_name));
    } catch (const std::exception& e) {
      PLOGE << "create connection for service " << service_name << " failed: " << e.what();
      throw;
    }
  }
};
}  // namespace dbus2http