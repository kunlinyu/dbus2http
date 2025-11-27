//
// Created by yukunlin on 2025/11/19.
//

#pragma once

#include <map>
#include <string>

#include "interface.h"
#include "object_path.h"

namespace dbus2http {

struct InterfaceContext {
  std::map<std::string, std::map<std::string, ObjectPath>> object_paths;
  std::map<std::string, Interface> interfaces;

  void add(const Interface& interface) {
    interfaces[interface.name] = interface;
  };

  void add(const std::string& service_name,
           const ObjectPath& object_path) {
    object_paths[service_name][object_path.path] = object_path;
  }



  bool contains_service(const std::string& service_name) const {
    return object_paths.contains(service_name);
  }

  const std::map<std::string, ObjectPath>& object_paths_of_service(
      const std::string& service_name) const {
    if (object_paths.contains(service_name))
      return object_paths.at(service_name);
    throw std::invalid_argument("service not found: " + service_name);
  }

  template <typename T>
  [[nodiscard]] const T& get(const std::string& interface_name,
                             const std::string& member_name) const {
    if (interfaces.contains(interface_name))
      return interfaces.at(interface_name).get<T>(member_name);
    throw std::invalid_argument("interface not found: " + interface_name);
  }
};

}  // namespace dbus2http
