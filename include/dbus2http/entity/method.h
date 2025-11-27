#pragma once

#include <string>
#include <vector>

#include "argument.h"

namespace dbus2http {
struct Method {
  std::string name;
  std::vector<Argument> args;
  Flags flags;

  Method() = default;

  void add(const Argument& arg) { args.push_back(arg); }

  [[nodiscard]] std::vector<Argument> out_args() const {
    std::vector<Argument> result;
    for (const auto& arg : args)
      if (arg.direction == "out") result.push_back(arg);
    return result;
  }
  [[nodiscard]] std::vector<Argument> in_args() const {
    std::vector<Argument> result;
    for (const auto& arg : args)
      if (arg.direction == "out") result.push_back(arg);
    return result;
  }
};

}  // namespace dbus2http
