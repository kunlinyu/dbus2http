//
// Created by yukunlin on 11/22/25.
//

#include <dbus2http/entity/DbusSerialization.h>

namespace dbus2http {

ObjectPath DbusSerialization::parse_single_object_path(
    const std::string& xml, const std::string& path,
    InterfaceContext& context) {
  tinyxml2::XMLDocument doc;
  doc.Parse(xml.c_str());
  const tinyxml2::XMLElement* root = doc.RootElement();
  return parse_single_object_path(root, path, context);
}

ObjectPath DbusSerialization::parse_single_object_path(
    const tinyxml2::XMLElement* node, const std::string& path,
    InterfaceContext& context) {
  ObjectPath object_path;
  if (!node) return object_path;

  object_path.path = path;

  // parse interfaces under this node
  for (const auto* iface = node->FirstChildElement("interface"); iface;
       iface = iface->NextSiblingElement("interface")) {
    object_path.add_interface(iface->Attribute("name"));
    Interface interface = parse_interface(iface);
    context.add(interface);
  }

  // parse child nodes
  for (const auto* child = node->FirstChildElement("node"); child;
       child = child->NextSiblingElement("node")) {
    const char* name_str = child->Attribute("name");
    if (name_str) object_path.add_childpath(name_str);
  }

  return object_path;
}

Interface DbusSerialization::parse_interface(
    const tinyxml2::XMLElement* iface_node) {
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

Method DbusSerialization::parse_method(
    const tinyxml2::XMLElement* method_node) {
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

Signal DbusSerialization::parse_signal(
    const tinyxml2::XMLElement* signal_node) {
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

Property DbusSerialization::parse_property(
    const tinyxml2::XMLElement* prop_node) {
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

Argument DbusSerialization::parse_argument(
    const tinyxml2::XMLElement* arg_node) {
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

}