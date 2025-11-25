#pragma once

#include <string>
#include <vector>
#include "argument.h"

namespace dbus2http {

struct Method {
  std::string name;
  std::vector<Argument> args;

  Method() = default;

  void add(const Argument& arg) { args.push_back(arg); }
};

}  // namespace dbus2http
