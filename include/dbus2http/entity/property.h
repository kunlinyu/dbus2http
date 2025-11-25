#pragma once

#include <string>

namespace dbus2http {

struct Property {
  std::string name;
  std::string type;
  std::string access;

  Property() = default;
  Property(const std::string& name, const std::string& type,
           const std::string& access)
      : name(name), type(type), access(access) {}
};

}  // namespace dbus2http
