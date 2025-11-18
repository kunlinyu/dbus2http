#pragma once

#include <string>
#include <map>
#include "object_path.h"

namespace dbus2http {

struct Service {
  std::string name;
  std::map<std::string, ObjectPath> object_paths;

  Service() = default;
  explicit Service(const std::string& name) : name(name) {}

  void add(const ObjectPath& op) { object_paths[op.path] = op; }
};

}  // namespace dbus2http
