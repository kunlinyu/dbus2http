#pragma once

#include <string>
#include <vector>
#include "interface.h"

namespace dbus2http {

struct ObjectPath {
  std::string path;
  std::vector<std::string> interfaces;
  std::vector<std::string> children_paths;

  ObjectPath() = default;
  explicit ObjectPath(const std::string& path) : path(path) {}

  void add_interface(const std::string& i) { interfaces.emplace_back(i); }
  void add_childpath(const std::string& p) { children_paths.emplace_back(p); }
};

}  // namespace dbus2http
