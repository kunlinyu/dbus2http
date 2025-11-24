//
// Created by yukunlin on 2025/11/17.
//

#pragma once

#include <string>
#include <vector>

#include "entity/DbusSerialization.h"
#include "entity/InterfaceContext.h"
#include "entity/service.h"

namespace dbus2http {

class DbusEnumerator {
  InterfaceContext& context_;

 public:
  DbusEnumerator(InterfaceContext& context) : context_(context) {}

  static std::vector<std::string> list_services();

  static std::string introspect_service(const std::string& service_name,
                                        const std::string& path);

  std::vector<ObjectPath> parse_object_paths_recursively(
      const std::string& service_name, const std::string& path) {
    const std::string xml = introspect_service(service_name, path);
    ObjectPath object = DbusSerialization::parse_single_object_path(xml, path, context_);
    std::vector<ObjectPath> result;
    result.emplace_back(object);
    std::cout << "  " << object.path << std::endl;
    for (const auto& child : object.children_paths) {
      std::string child_path = path + (path == "/" ? "" : "/") + child;
      std::vector<ObjectPath> child_ops =
          parse_object_paths_recursively(service_name, child_path);
      result.insert(result.end(), child_ops.begin(), child_ops.end());
    }
    return result;
  }
};

}  // namespace dbus2http
