#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "dbus2http-proxy/DbusEnumerator.h"
#include "dbus2http-proxy/entity/DbusSerialization.h"

int main() {
  std::cout << "Hello, World!" << std::endl;
  nlohmann::json json;
  json.emplace("hello", "world");
  std::cout << json.dump() << std::endl;

  dbus2http::DbusEnumerator dbusEnumerator;
  auto service_names = dbus2http::DbusEnumerator::list_services();
  int i = 0;
  nlohmann::json all;
  for (const auto& service_name : service_names) {
    std::cout << "=====" << service_name << "===== " << i++ << std::endl;
    std::vector<dbus2http::ObjectPath> object_paths =
        dbus2http::DbusSerialization::parse_object_paths_recursively(
            service_name, "/");
    object_paths.erase(std::remove_if(object_paths.begin(), object_paths.end(),
                            [](const dbus2http::ObjectPath& op) {
                              return op.interfaces.empty();
                            }),
             object_paths.end());
    nlohmann::json j;
    for (const auto& op : object_paths) {
      j[op.path] = op;
    }
    all[service_name] = j;
  }

  // save all into a file
  std::ofstream fout("dbus_structure.json");
  if (fout.is_open()) fout << all.dump(2);
  fout.close();

  return 0;
}
