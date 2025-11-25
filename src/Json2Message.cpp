//
// Created by yukunlin on 11/22/25.
//
#include <dbus2http-proxy/Json2Message.h>
#include <dbus2http-proxy/SignatureUtils.h>

namespace dbus2http {

void Json2Message::FillMethod(sdbus::MethodCall& method_call,
                              const Method& method_type,
                              const nlohmann::json& json) {
  for (const auto& arg : method_type.args) {
    if (arg.direction != "in") continue;
    if (!json.contains(arg.name))
      throw std::invalid_argument("Missing argument: " + arg.name);
    FillMethodSig(method_call, json[arg.name], arg.type);
  }
}

void Json2Message::FillMethodSig(sdbus::MethodCall& method_call,
                                 const nlohmann::json& json,
                                 const std::string& sig) {
  PLOGD << "FillmethodSig: " << json.dump() << " " << sig;
  std::vector<std::string> complete_sigs = SignatureUtils::split(sig);
  if (complete_sigs.size() > 1) {
    if (not json.is_array()) {
      std::string err_msg =
          "Expected array type but we get :" + std::string(json.type_name());
      PLOGE << err_msg;
      throw std::invalid_argument(err_msg);
    }
    if (json.size() != complete_sigs.size())
      throw std::invalid_argument(
          "Expected array size " + std::to_string(complete_sigs.size()) +
          " but we get :" + std::to_string(json.size()));
    for (size_t i = 0; i < complete_sigs.size(); ++i)
      FillMethodSig(method_call, json[i], complete_sigs[i]);
  } else if (complete_sigs.size() == 1) {
    const std::string& current_sig = complete_sigs.front();
    switch (current_sig.front()) {
      case 'b':  // boolean
        if (json.is_boolean()) {
          PLOGD << "append bool " << json.dump();
          method_call << json.get<bool>();
        } else
          throw std::invalid_argument("Expected bool type but we get :" +
                                      std::string(json.type_name()));
        break;
      case 'y':  // byte
        AppendIntegerFromJson<uint8_t>(method_call, json);
        break;
      case 'n':  // int16
        AppendIntegerFromJson<int16_t>(method_call, json);
        break;
      case 'q':  // uint16
        AppendIntegerFromJson<uint16_t>(method_call, json);
        break;
      case 'i':  // int32
        AppendIntegerFromJson<int32_t>(method_call, json);
        break;
      case 'u':  // uint32
        AppendIntegerFromJson<uint32_t>(method_call, json);
        break;
      case 'x':  // int64
        AppendIntegerFromJson<int64_t>(method_call, json);
        break;
      case 't':  // uint64
        AppendIntegerFromJson<uint64_t>(method_call, json);
        break;
      case 'd':  // double
        if (json.is_number()) {
          PLOGD << "append double " << json.dump();
          method_call << json.get<double>();
        } else
          throw std::invalid_argument("Expected float type but we get :" +
                                      std::string(json.type_name()));
        break;
      case 's':  // string
        if (json.is_string()) {
          PLOGD << "append string " << json.dump();
          method_call << json.get<std::string>();
        } else
          throw std::invalid_argument("Expected float type but we get :" +
                                      std::string(json.type_name()));
        break;
      case 'v':  // variant
        FillVariant(method_call, json);
        break;
      case 'a':  // array, dict
        if (json.is_array() || json.is_object()) {
          std::string array_sig = current_sig.substr(1);
          std::string element_sig = array_sig.substr(1, array_sig.size() - 2);
          PLOGD << "open container: " << array_sig;
          method_call.openContainer(array_sig.c_str());
          PLOGD << "container opened";

          if (current_sig[1] == '{')  // dict
            for (const auto& [key, value] : json.items()) {
              PLOGD << "open dict entry " << element_sig;
              method_call.openDictEntry(element_sig.c_str());

              PLOGD << "dict opened";
              PLOGD << "extract key " << key;
              PLOGD << "extract " << value.dump() << " from " << json.dump();
              FillMethodSig(method_call, key, element_sig.substr(0, 1));
              FillMethodSig(method_call, value, element_sig.substr(1));
              PLOGD << "close dict entry " << element_sig;
              method_call.closeDictEntry();
              PLOGD << "dict closed";
            }
          else if (current_sig[1] == '(')  // array
            for (const auto& j : json) FillMethodSig(method_call, j, array_sig);
          else
            for (const auto& j : json) FillMethodSig(method_call, j, array_sig);
          PLOGD << "close container: " << array_sig;
          method_call.closeContainer();
        } else {
          std::string err_msg = "Expected array type but we get :" +
                                std::string(json.type_name());
          PLOGE << err_msg;
          throw std::invalid_argument(err_msg);
        }

        break;
      case '(':  // struct
      {
        std::string element_sig = current_sig.substr(1, current_sig.size() - 2);
        PLOGD << "open struct " << element_sig;
        method_call.openStruct(element_sig.c_str());
        FillMethodSig(method_call, json, element_sig);
        PLOGD << "close struct " << current_sig;
        method_call.closeStruct();
      } break;
      default:;
    }
  }
}
void Json2Message::FillVariant(sdbus::MethodCall& method_call,
                               const nlohmann::json& json) {
  if (not json.is_object())
    throw std::invalid_argument("Expected object type for variant");
  if (not json.contains("variant") or not json.contains("value"))
    throw std::invalid_argument("Missing signature or value for variant");
  std::string sig = json["variant"].get<std::string>();
  PLOGD << "open variant " << sig;
  method_call.openVariant(sig.c_str());
  FillMethodSig(method_call, json["value"], sig);
  PLOGD << "close variant " << sig;
  method_call.closeVariant();
}

}  // namespace dbus2http