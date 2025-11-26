#pragma once

#include <string>
#include <vector>

#include "method.h"
#include "property.h"
#include "signal.h"

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

  const Method& get_method(const std::string& method_name) const {
    for (const auto& m : methods)
      if (m.name == method_name) return m;
    throw std::invalid_argument("Method not found: " + method_name);
  }
  const Signal& get_signal(const std::string& signal_name) const {
    for (const auto& s : signals)
      if (s.name == signal_name) return s;
    throw std::invalid_argument("Signal not found: " + signal_name);
  }
  const Property& get_property(const std::string& property_name) const {
    for (const auto& p : properties)
      if (p.name == property_name) return p;
    throw std::invalid_argument("Property not found: " + property_name);
  }
};

}  // namespace dbus2http
