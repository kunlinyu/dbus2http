#pragma once

#include <string>
#include <vector>
#include "method.h"
#include "signal.h"
#include "property.h"

namespace dbus2http {

struct Interface {
  std::string name;
  std::vector<Method> methods;
  std::vector<Signal> signals;
  std::vector<Property> properties;

  Interface() = default;
  explicit Interface(const std::string& name) : name(name) {}

  void add(const Method& m) { methods.push_back(m); }
  void add(const Signal& s) { signals.push_back(s); }
  void add(const Property& p) { properties.push_back(p); }
};

}  // namespace dbus2http
