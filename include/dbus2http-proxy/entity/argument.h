#pragma once

#include <string>

namespace entity {

struct Argument {
  std::string name;
  std::string type;
  std::string direction;

  Argument() = default;
  Argument(const std::string& name, const std::string& type,
           const std::string& direction)
      : name(name), type(type), direction(direction) {}
};

}  // namespace entity
