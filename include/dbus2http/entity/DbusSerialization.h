//
// Created by yukunlin on 2025/11/17.
//

#pragma once

#include <httplib.h>
#include <tinyxml2.h>

#include <string>
#include <nlohmann/json.hpp>


#include "argument.h"
#include "interface.h"
#include "method.h"
#include "object_path.h"
#include "property.h"
#include "service.h"
#include "signal.h"
#include "InterfaceContext.h"

namespace dbus2http {

// serialization to json
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Argument, name, type, direction);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Method, name, args);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Signal, name, args);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Property, name, type, access);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectPath, interfaces);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Service, name, object_paths);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Interface, name, methods, signals,
                                   properties);

// deserialization from xml
class DbusSerialization {
 public:
  static ObjectPath parse_single_object_path(const std::string& xml,
                                             const std::string& path,
                                             InterfaceContext& context);

 private:
  static ObjectPath parse_single_object_path(const tinyxml2::XMLElement* node,
                                             const std::string& path,
                                             InterfaceContext& context);

  static Interface parse_interface(const tinyxml2::XMLElement* iface_node);

  static Method parse_method(const tinyxml2::XMLElement* method_node);

  static Signal parse_signal(const tinyxml2::XMLElement* signal_node);

  static Property parse_property(const tinyxml2::XMLElement* prop_node);

  static Argument parse_argument(const tinyxml2::XMLElement* arg_node);
};

}  // namespace dbus2http
