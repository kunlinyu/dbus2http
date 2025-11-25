#pragma once

#include <string>
#include <vector>
#include "argument.h"

namespace dbus2http {

struct Signal {
  std::string name;
  std::vector<Argument> args;

  Signal() = default;
  explicit Signal(const std::string& name) : name(name) {}

  void add(const Argument& arg) { args.push_back(arg); }
};

}  // namespace dbus2http
