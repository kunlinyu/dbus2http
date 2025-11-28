//
// Created by yukunlin on 2025/11/17.
//

#pragma once

#include <httplib.h>
#include <tinyxml2.h>

#include <nlohmann/json.hpp>
#include <string>

#include "InterfaceContext.h"
#include "argument.h"
#include "flags.h"
#include "interface.h"
#include "method.h"
#include "object_path.h"
#include "property.h"
#include "service.h"
#include "signal.h"

namespace dbus2http {
// serialization to json
NLOHMANN_JSON_SERIALIZE_ENUM(EmitsChangedSignal, {{TRUE, "true"},
                                                  {FALSE, "false"},
                                                  {INVALIDATES, "invalidates"},
                                                  {CONST, "const"}});
template <typename BasicJsonType,
          nlohmann::detail::enable_if_t<
              nlohmann::detail::is_basic_json<BasicJsonType>::value, int> = 0>
void to_json(BasicJsonType& j, const Flags& flags) {
  if (flags.deprecated) j["deprecated"] = flags.deprecated;
  if (flags.method_no_reply) j["method_no_reply"] = flags.method_no_reply;
  if (flags.privileged) j["privileged"] = flags.privileged;
  if (flags.emits_changed_signal != TRUE)
    j["emits_changed_signal"] = flags.emits_changed_signal;
  if (!flags.c_symbol.empty()) j["c_symbol"] = flags.c_symbol;
}

template <typename BasicJsonType,
          nlohmann::detail::enable_if_t<
              nlohmann::detail::is_basic_json<BasicJsonType>::value, int> = 0>
void from_json(const BasicJsonType& j, Flags& flags) {
  if (j.contains("deprecated")) j.at("deprecated").get_to(flags.deprecated);
  if (j.contains("method_no_reply"))
    j.at("method_no_reply").get_to(flags.method_no_reply);
  if (j.contains("privileged")) j.at("privileged").get_to(flags.privileged);
  if (j.contains("emits_changed_signal"))
    j.at("emits_changed_signal").get_to(flags.emits_changed_signal);
  if (j.contains("c_symbol")) j.at("c_symbol").get_to(flags.c_symbol);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Argument, name, type, direction, flags);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Method, name, args, flags);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Signal, name, args, flags);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Property, name, type, access, flags);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectPath, interfaces);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Service, name, object_paths);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Interface, name, methods, signals,
                                   properties, flags);

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

  static Flags parse_flags(const tinyxml2::XMLElement* parent_node);
};

class Dbus2Html {
 public:
  static std::string to_html(
      const std::map<std::string, std::map<std::string, ObjectPath>>&
          object_paths);
  static std::string to_html(const ObjectPath& op, const std::string& service_name = "");

  static std::string to_html(const Interface& interface);

  static std::string to_html(const Method& method);

  static std::string to_html(const Signal& signal);

  static std::string to_html(const Property& property);

  static std::string to_html(const Flags& flags);
};

}  // namespace dbus2http
