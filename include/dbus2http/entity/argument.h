#pragma once

#include <string>

#include "flags.h"

namespace dbus2http {

struct Argument {
  std::string name;
  std::string type;
  std::string direction;
  Flags flags;

  Argument() = default;
  Argument(const std::string& name, const std::string& type,
           const std::string& direction)
      : name(name), type(type), direction(direction) {}
};

}  // namespace dbus2http
