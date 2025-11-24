//
// Created by yukunlin on 2025/11/19.
//

#pragma once

#include <map>
#include <string>

#include "interface.h"
#include "method.h"
#include "property.h"
#include "signal.h"
#include "object_path.h"

namespace dbus2http {

struct InterfaceContext {
  std::map<std::string, std::map<std::string, ObjectPath>> object_paths;
  std::map<std::string, std::map<std::string, Method>> methods;
  std::map<std::string, std::map<std::string, Signal>> signals;
  std::map<std::string, std::map<std::string, Property>> properties;
  std::map<std::string, Interface> interfaces;

  void add(const Interface& interface) {
    interfaces[interface.name] = interface;
    for (const auto& method : interface.methods)
      methods[interface.name][method.name] = method;
    for (const auto& signal : interface.signals)
      signals[interface.name][signal.name] = signal;
    for (const auto& property : interface.properties)
      properties[interface.name][property.name] = property;
  };

  const Method& GetMethod(const std::string& interface_name,
                          const std::string& method_name) const {
    std::invalid_argument e("Method not found: " + interface_name + " " +
                            method_name);
    if (!methods.contains(interface_name)) throw e;
    const auto& interface = methods.at(interface_name);
    if (!interface.contains(method_name)) throw e;
    return interface.at(method_name);
  }
};

}  // namespace dbus2http
