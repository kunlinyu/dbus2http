//
// Created by yukunlin on 11/22/25.
//

#include <dbus2http/entity/DbusSerialization.h>
#include <plog/Log.h>

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
  Interface interface;
  if (!iface_node) return interface;

  const char* name_attr = iface_node->Attribute("name");
  if (name_attr) interface.name = name_attr;

  for (const auto* method = iface_node->FirstChildElement("method"); method;
       method = method->NextSiblingElement("method")) {
    interface.add(parse_method(method));
  }

  for (const auto* signal = iface_node->FirstChildElement("signal"); signal;
       signal = signal->NextSiblingElement("signal")) {
    interface.add(parse_signal(signal));
  }

  for (const auto* prop = iface_node->FirstChildElement("property"); prop;
       prop = prop->NextSiblingElement("property")) {
    interface.add(parse_property(prop));
  }

  interface.flags = parse_flags(iface_node);

  return interface;
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
  m.flags = parse_flags(method_node);
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
  s.flags = parse_flags(signal_node);
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

  p.flags = parse_flags(prop_node);
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

  a.flags = parse_flags(arg_node);
  return a;
}

Flags DbusSerialization::parse_flags(const tinyxml2::XMLElement* parent_node) {
  Flags flags;
  if (!parent_node) return flags;
  for (const auto* annotation = parent_node->FirstChildElement("annotation");
       annotation; annotation = annotation->NextSiblingElement("annotation")) {
    const char* name_attr = annotation->Attribute("name");
    const char* value_attr = annotation->Attribute("value");
    if (name_attr == nullptr || value_attr == nullptr) {
      PLOGW << "annotation missing name or value attribute";
      continue;
    }
    std::string name = std::string(name_attr);
    std::string value = std::string(value_attr);
    if (name == "org.freedesktop.DBus.Deprecated") {
      if (value == "true")
        flags.deprecated = true;
      else if (value == "false")
        flags.deprecated = false;
      else
        PLOGW << "unrecognized value for annotation "
                 "org.freedesktop.DBus.Deprecated: "
              << value_attr;
    } else if (name == "org.freedesktop.DBus.Method.NoReply") {
      if (value == "true")
        flags.method_no_reply = true;
      else if (value == "false")
        flags.method_no_reply = false;
      else
        PLOGW << "unrecognized value for annotation "
                 "org.freedesktop.DBus.Method.NoReply: "
              << value_attr;
    } else if (name == "org.freedesktop.DBus.Property.EmitsChangedSignal") {
      if (value == "true")
        flags.emits_changed_signal = TRUE;
      else if (value == "false")
        flags.emits_changed_signal = FALSE;
      else if (value == "invalidates")
        flags.emits_changed_signal = INVALIDATES;
      else if (value == "const")
        flags.emits_changed_signal = CONST;
      else
        PLOGW << "unrecognized value for annotation "
                 "org.freedesktop.DBus.Property.EmitsChangedSignal: "
              << value_attr;
    } else if (name == "org.freedesktop.DBus.GLib.CSymbol") {
      flags.c_symbol = value_attr;
    } else {
      PLOGW << "unrecognized annotation: " << name_attr;
    }
  }
  return flags;
}
std::string Dbus2Html::to_html(
    const std::map<std::string, std::map<std::string, ObjectPath>>&
        object_paths) {
  std::ostringstream oss;
  for (const auto& [service_name, paths] : object_paths) {
    oss << "<details>";
    oss << "<summary>" << service_name << "</summary>";
    for (const auto& [path, op] : paths) oss << to_html(op, service_name);
    oss << "</details>";
  }
  return oss.str();
}
std::string Dbus2Html::to_html(const ObjectPath& op,
                               const std::string& service_name) {
  std::ostringstream oss;
  oss << "<details>";
  oss << "<summary>" << op.path << "</summary>";
  for (const auto& interface : op.interfaces) {
    oss << "<p>" << "<a target=\"_blank\" href=\"" << "/dbus/interface/html/"
        << interface;
    oss << "?object_path=" << op.path;
    oss << "&interface_name=" << interface;
    if (not service_name.empty()) oss << "&service_name=" << service_name;
    oss << "\">" << interface << "</a>" << "</p>";
  }
  oss << "</details>";
  return oss.str();
}
std::string Dbus2Html::to_html(const Interface& interface) {
  std::ostringstream oss;
  oss << "<details open>";
  oss << "<summary>" << interface.name << "</summary>";
  oss << "<details open><summary>Methods</summary>";
  for (const auto& [method_name, method] : interface.methods) {
    oss << to_html(method);
  }
  oss << "</details>";
  oss << "<details open><summary>Signals</summary>";
  for (const auto& [signal_name, signal] : interface.signals) {
    oss << to_html(signal);
  }
  oss << "</details>";
  oss << "<details open><summary>Properties</summary>";
  for (const auto& [property_name, property] : interface.properties) {
    oss << to_html(property);
  }
  oss << "</details>";
  oss << "</details>";
  std::string try_jump = R"(
  <script>
    const params = new URLSearchParams(window.location.search);
    const object_path = params.get('object_path');
    const service_name = params.get('service_name');
    const interface_name = params.get('interface_name');
    const dynamicLinks = document.querySelectorAll('.try-method');
    dynamicLinks.forEach(link => {
      const method = link.textContent.trim();
      link.href = `/dbus/try/${service_name}${object_path}/${interface_name}.${method}`;
    });
  </script>
)";
  oss << try_jump;
  return oss.str();
}
std::string Dbus2Html::to_html(const Method& method) {
  std::ostringstream oss;
  oss << "<details>";
  oss << "<summary>";
  oss << "<a class=\"try-method\" target=\"_blank\">" << method.name << "</a>";
  oss << to_html(method.flags);
  oss << "</summary>";
  for (const auto& arg : method.args) {
    oss << "<p>" << arg.name << "[" << arg.direction << "]: " << arg.type
        << "</p>";
  }
  oss << "</details>";
  return oss.str();
}
std::string Dbus2Html::to_html(const Signal& signal) {
  std::ostringstream oss;
  oss << "<details>";
  oss << "<summary>";
  oss << "<a target=\"_blank\" href=\"" << "/dbus/matchrule";
  oss << "?member=" << signal.name;
  oss << "\">";
  oss << signal.name;
  oss << "</a>";
  oss << to_html(signal.flags);
  oss << "</summary>";
  for (const auto& arg : signal.args) {
    oss << "<p>" << arg.name << ": " << arg.name << "</p>";

  }
  oss << "</details>";
  return oss.str();
}
std::string Dbus2Html::to_html(const Property& property) {
  std::ostringstream oss;
  oss << "<p>" << property.name << "[" << property.access << "]"
      << to_html(property.flags) << " : " << property.type << "</p>";
  return oss.str();
}
std::string Dbus2Html::to_html(const Flags& flags) {
  std::ostringstream oss;
  if (flags.deprecated) oss << "(deprecated)";
  if (flags.method_no_reply) oss << "(no reply)";
  if (flags.privileged) oss << "(privileged)";
  return oss.str();
}

}  // namespace dbus2http