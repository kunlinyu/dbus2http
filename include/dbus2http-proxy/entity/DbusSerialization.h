//
// Created by yukunlin on 2025/11/17.
//

#pragma once

#include <dbus2http-proxy/DbusEnumerator.h>
#include <sdbus-c++/Message.h>
#include <tinyxml2.h>

#include <string>

#include "argument.h"
#include "interface.h"
#include "method.h"
#include "object_path.h"
#include "property.h"
#include "service.h"
#include "signal.h"

namespace dbus2http {

// serialization to json
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Argument, name, type, direction);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Interface, name, methods, signals,
                                   properties);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Method, name, args);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Signal, name, args);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Property, name, type, access);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectPath, interfaces);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Service, name, object_paths);

// deserialization from xml
class DbusSerialization {
 public:
  static std::vector<ObjectPath> parse_object_paths_recursively(
      const std::string& service_name, const std::string& path) {
    const std::string xml =
        DbusEnumerator::introspect_service(service_name, path);
    tinyxml2::XMLDocument doc;
    doc.Parse(xml.c_str());
    const tinyxml2::XMLElement* root = doc.RootElement();
    ObjectPath op = parse_single_object_path(root, path);
    std::vector<ObjectPath> result;
    result.push_back(op);
    for (const auto& child : op.children_paths) {
      std::string child_path = path + (path == "/" ? "" : "/") + child;
      std::vector<ObjectPath> child_ops =
          parse_object_paths_recursively(service_name, child_path);
      result.insert(result.end(), child_ops.begin(), child_ops.end());
    }
    return result;
  }

 private:
  static ObjectPath parse_single_object_path(const tinyxml2::XMLElement* node,
                                             const std::string& path) {
    ObjectPath object_path;
    if (!node) return object_path;

    object_path.path = path;

    // parse interfaces under this node
    for (const auto* iface = node->FirstChildElement("interface"); iface;
         iface = iface->NextSiblingElement("interface"))
      object_path.add_interface(iface->Attribute("name"));

    // parse child nodes
    for (const auto* child = node->FirstChildElement("node"); child;
         child = child->NextSiblingElement("node")) {
      const char* name_str = child->Attribute("name");
      if (name_str) object_path.add_childpath(name_str);
    }

    return object_path;
  }

  static Interface parse_interface(const tinyxml2::XMLElement* iface_node) {
    Interface iface;
    if (!iface_node) return iface;

    const char* name_attr = iface_node->Attribute("name");
    if (name_attr) iface.name = name_attr;

    for (const auto* method = iface_node->FirstChildElement("method"); method;
         method = method->NextSiblingElement("method")) {
      iface.add(parse_method(method));
    }

    for (const auto* signal = iface_node->FirstChildElement("signal"); signal;
         signal = signal->NextSiblingElement("signal")) {
      iface.add(parse_signal(signal));
    }

    for (const auto* prop = iface_node->FirstChildElement("property"); prop;
         prop = prop->NextSiblingElement("property")) {
      iface.add(parse_property(prop));
    }

    return iface;
  }

  static Method parse_method(const tinyxml2::XMLElement* method_node) {
    Method m;
    if (!method_node) return m;

    const char* name_attr = method_node->Attribute("name");
    if (name_attr) m.name = name_attr;

    for (const auto* arg = method_node->FirstChildElement("arg"); arg;
         arg = arg->NextSiblingElement("arg")) {
      m.add(parse_argument(arg));
    }
    return m;
  }

  static Signal parse_signal(const tinyxml2::XMLElement* signal_node) {
    Signal s;
    if (!signal_node) return s;

    const char* name_attr = signal_node->Attribute("name");
    if (name_attr) s.name = name_attr;

    for (const auto* arg = signal_node->FirstChildElement("arg"); arg;
         arg = arg->NextSiblingElement("arg")) {
      s.add(parse_argument(arg));
    }
    return s;
  }

  static Property parse_property(const tinyxml2::XMLElement* prop_node) {
    Property p;
    if (!prop_node) return p;

    const char* name_attr = prop_node->Attribute("name");
    const char* type_attr = prop_node->Attribute("type");
    const char* access_attr = prop_node->Attribute("access");

    if (name_attr) p.name = name_attr;
    if (type_attr) p.type = type_attr;
    if (access_attr) p.access = access_attr;
    return p;
  }

  static Argument parse_argument(const tinyxml2::XMLElement* arg_node) {
    Argument a;
    if (!arg_node) return a;

    const char* name_attr = arg_node->Attribute("name");
    const char* type_attr = arg_node->Attribute("type");
    const char* dir_attr = arg_node->Attribute("direction");

    if (name_attr) a.name = name_attr;
    if (type_attr) a.type = type_attr;
    if (dir_attr) a.direction = dir_attr;
    return a;
  }
};

}  // namespace dbus2http
