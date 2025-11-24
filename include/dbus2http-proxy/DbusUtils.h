//
// Created by yukunlin on 2025/11/24.
//

#pragma onece
#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/Types.h>

namespace dbus2http {
class DbusUtils {
 public:
  static std::unique_ptr<sdbus::IConnection> createConnection(bool system_bus) {
    if (system_bus) return sdbus::createSystemBusConnection();
    return sdbus::createSessionBusConnection();
  }
  static std::unique_ptr<sdbus::IConnection> createConnection(
      const std::string& service_name, bool system_bus) {
    if (system_bus)
      return sdbus::createSystemBusConnection(sdbus::ServiceName(service_name));
    return sdbus::createSessionBusConnection(sdbus::ServiceName(service_name));
  }
};
}  // namespace dbus2http