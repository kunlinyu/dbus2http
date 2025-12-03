//
// Created by yukunlin on 2025/11/17.
//

#include <dbus2http/DbusEnumerator.h>
#include <sdbus-c++/sdbus-c++.h>

#include <algorithm>
#include <exception>
#include <string>
#include <vector>

#include "dbus2http/DbusUtils.h"

namespace dbus2http {

std::vector<std::string> DbusEnumerator::list_services(bool system_bus) {
  try {
    auto connection = DbusUtils::createConnection(system_bus);

    auto proxy = sdbus::createProxy(*connection,
                                    sdbus::ServiceName("org.freedesktop.DBus"),
                                    sdbus::ObjectPath("/org/freedesktop/DBus"));

    std::vector<std::string> names;
    proxy->callMethod("ListNames")
        .onInterface("org.freedesktop.DBus")
        .storeResultsTo(names);

    // Remove bus unique names that start with ":1"
    names.erase(std::remove_if(names.begin(), names.end(), [](const auto& s) {
      return !s.empty() && s.rfind(":1", 0) == 0;
    }), names.end());


    return names;
  } catch (const sdbus::Error&) {
    return {};
  } catch (const std::exception&) {
    return {};
  }
}

std::string DbusEnumerator::introspect_service(sdbus::IConnection& conn, const std::string& service_name, const std::string& path) {
  try {
    std::unique_ptr<sdbus::IProxy> proxy = sdbus::createProxy(
        conn, sdbus::ServiceName(service_name), sdbus::ObjectPath(path));

    std::string xml;
    proxy->callMethod("Introspect")
        .onInterface("org.freedesktop.DBus.Introspectable")
        .storeResultsTo(xml);
    return xml;
  } catch (const sdbus::Error&) {
    return {};
  } catch (const std::exception&) {
    return {};
  }
}

}  // namespace dbus2http
