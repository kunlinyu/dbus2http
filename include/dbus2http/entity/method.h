#pragma once

#include <string>
#include <vector>
#include "argument.h"

namespace dbus2http {

struct Method {
  std::string name;
  std::vector<Argument> args;

  Method() = default;
  explicit Method(const std::string& name) : name(name) {}

  void add(const Argument& arg) { args.push_back(arg); }
};

}  // namespace dbus2http
