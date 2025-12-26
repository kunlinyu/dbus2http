//
// Created by yukunlin on 2025/11/19.
//

#pragma once

#include <map>
#include <string>
#include <mutex>

#include "interface.h"
#include "object_path.h"

namespace dbus2http {

class InterfaceContext {
  std::map<std::string, std::map<std::string, ObjectPath>> object_paths;
  std::map<std::string, Interface> interfaces;
  mutable std::recursive_mutex mutex;

 public:
  void add(const Interface& interface) {
    std::unique_lock lock(mutex);
    interfaces[interface.name] = interface;
  };

  void add(const std::string& service_name,
           const ObjectPath& object_path) {
    std::unique_lock lock(mutex);
    object_paths[service_name][object_path.path] = object_path;
  }

  Interface get_interfaces(const std::string& interface_name) const {
    std::unique_lock lock(mutex);
    return interfaces.at(interface_name);
  }

  ObjectPath get_object_path(const std::string& service_name, const std::string& path_string) const {
    std::unique_lock lock(mutex);
    return object_paths.at(service_name).at(path_string);
  }

  std::vector<std::string> get_service_names() const {
    std::unique_lock lock(mutex);
    std::vector<std::string> service_names;
    for (const auto& [service_name, _] : object_paths)
      service_names.push_back(service_name);
    return service_names;
  }

  std::vector<std::string> get_object_path_string(const std::string& service_name) const {
    std::unique_lock lock(mutex);
    std::vector<std::string> object_path_strings;
    if (not contains_service(service_name)) return object_path_strings;
    for (const auto& [path, object_path] : object_paths.at(service_name))
      object_path_strings.push_back(path);
    return object_path_strings;
  }

  bool contains_service(const std::string& service_name) const {
    std::unique_lock lock(mutex);
    return object_paths.contains(service_name);
  }

  bool contains_interface(const std::string& interface_name) const {
    std::unique_lock lock(mutex);
    return interfaces.contains(interface_name);
  }

  const std::map<std::string, ObjectPath>& object_paths_of_service(
      const std::string& service_name) const {
    std::unique_lock lock(mutex);
    if (object_paths.contains(service_name))
      return object_paths.at(service_name);
    throw std::invalid_argument("service not found: " + service_name);
  }

  template <typename T>
  [[nodiscard]] const T& get(const std::string& interface_name,
                             const std::string& member_name) const {
    std::unique_lock lock(mutex);
    if (interfaces.contains(interface_name))
      return interfaces.at(interface_name).get<T>(member_name);
    throw std::invalid_argument("interface not found: " + interface_name);
  }
};

}  // namespace dbus2http
