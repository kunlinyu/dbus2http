#pragma once

#include <string>
#include <vector>
#include "interface.h"

namespace entity {

struct ObjectPath {
  std::string path;
  std::vector<Interface> interfaces;
  std::vector<ObjectPath> children;

  ObjectPath() = default;
  explicit ObjectPath(const std::string& path) : path(path) {}

  void add(const Interface& i) { interfaces.push_back(i); }
  void add(const ObjectPath& c) { children.push_back(c); }
};

}  // namespace entity
