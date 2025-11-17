#include <iostream>
#include <nlohmann/json.hpp>

#include "dbus2http-proxy/DbusEnumerator.h"

int main() {
  std::cout << "Hello, World!" << std::endl;
  nlohmann::json json;
  json.emplace("hello", "world");
  std::cout << json.dump() << std::endl;

  dbus2http::DbusEnumerator dbusEnumerator;
  auto service_names = dbus2http::DbusEnumerator::list_services();
  int i = 0;
  for (const auto& service_name : service_names) {
    std::cout << "=====" << service_name << "===== " << i++ << std::endl;
    std::cout << dbus2http::DbusEnumerator::introspect_service(service_name) << std::endl;
  }

  return 0;
}
