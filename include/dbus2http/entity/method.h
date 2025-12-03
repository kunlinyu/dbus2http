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

  void add(const Argument& arg) {
    Argument argument = arg;
    if (argument.name.empty())
      argument.name = "arg" + std::to_string(args.size());
    args.push_back(argument);
  }

  [[nodiscard]] std::string in_signature() const {
    std::ostringstream result;
    for (const auto& arg : args)
      if (arg.direction == "in") result << arg.type;
    return result.str();
  }

  [[nodiscard]] std::string out_signature() const {
    std::ostringstream result;
    for (const auto& arg : args)
      if (arg.direction == "out") result << arg.type;
    return result.str();
  }

  [[nodiscard]] std::vector<Argument> out_args() const {
    std::vector<Argument> result;
    for (const auto& arg : args)
      if (arg.direction == "out") result.push_back(arg);
    return result;
  }
  [[nodiscard]] std::vector<Argument> in_args() const {
    std::vector<Argument> result;
    for (const auto& arg : args)
      if (arg.direction == "in") result.push_back(arg);
    return result;
  }
};

}  // namespace dbus2http
