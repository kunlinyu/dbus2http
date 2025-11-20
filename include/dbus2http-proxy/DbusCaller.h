//
// Created by yukunlin on 2025/11/18.
//
#pragma once

#include <sdbus-c++/IConnection.h>
#include <sdbus-c++/IProxy.h>
#include <sdbus-c++/Types.h>

#include <nlohmann/json.hpp>
#include <string>

#include "entity/InterfaceContext.h"
#include "entity/method.h"

namespace dbus2http {

class DbusCaller {
 private:
  std::unique_ptr<sdbus::IConnection> conn_;
  const InterfaceContext& context_;

 public:
  DbusCaller(const InterfaceContext& context) : context_(context) {
    conn_ = sdbus::createSessionBusConnection();
  }

  void Call(const std::string& service_name, const std::string& object_path,
            const std::string& interface_name, const std::string& method_name,
            const nlohmann::json& request) {
    auto proxy = sdbus::createProxy(sdbus::ServiceName(service_name),
                                    sdbus::ObjectPath(object_path));
    auto method = proxy->createMethodCall(sdbus::InterfaceName(interface_name),
                                          sdbus::MethodName(method_name));
    FillMethod(method, context_.GetMethod(interface_name, method_name),
               request);

    auto reply = proxy->callMethod(method);
  }

 private:
  void FillMethod(sdbus::MethodCall& method_call, const Method& method_type,
                  const nlohmann::json& json) {
    for (const auto& arg : method_type.args) {
      if (arg.direction == "out") continue;
      if (!json.contains(arg.name))
        throw std::invalid_argument("Missing argument: " + arg.name);
      FillMethodSig(method_call, json[arg.name], arg.type);
    }
  }

  template <typename T>
  T extract_integer(const std::string& key, char sig) {
    long long integer = std::stoll(key);
    if (integer >= std::numeric_limits<T>::min() &&
        integer <= std::numeric_limits<T>::max())
      return static_cast<T>(integer);
    throw std::invalid_argument(
        key + "Exceeds the valid range of target type " + sig);
  }

  void FillMethodSig(sdbus::MethodCall& method_call, const std::string& key,
                     const nlohmann::json& json, const std::string& sig) {
    if (sig.size() != 2)
      throw std::invalid_argument("Expected signature length 2 but we get :" +
                                  sig);
    switch (const char key_type = sig.front()) {
      case 'b':  // boolean
        if (key == "true")
          method_call << true;
        else if (key == "false")
          method_call << false;
        else
          throw std::invalid_argument("Expected boolean key but we get: " +
                                      key);
        break;
      case 'y':  // byte
        method_call << extract_integer<uint8_t>(key, key_type);
        break;
      case 'n':  // int16
        method_call << extract_integer<int16_t>(key, key_type);
        break;
      case 'q':  // uint16
        method_call << extract_integer<uint16_t>(key, key_type);
        break;
      case 'i':  // int32
        method_call << extract_integer<int32_t>(key, key_type);
        break;
      case 'u':  // uint32
        method_call << extract_integer<uint32_t>(key, key_type);
        break;
      case 'x':  // int64
        method_call << extract_integer<int64_t>(key, key_type);
        break;
      case 't':  // uint64
        method_call << extract_integer<uint64_t>(key, key_type);
        break;
      case 'd':  // double
      {
        double value = std::stod(key);
        method_call << value;
      } break;
      case 's':  // string
        method_call << key;
        break;
      default:
        throw std::invalid_argument(
            "the key of dict must be basic type but we get: " + sig);
    }
    FillMethodSig(method_call, json, sig.substr(1));
  }
  void FillMethodSig(sdbus::MethodCall& method_call, const nlohmann::json& json,
                     const std::string& sig) {
    std::cout << "FillmethodSig: " << json.dump() << std::endl;
    std::vector<std::string> complete_sigs = split_signature(sig);
    if (complete_sigs.size() > 1) {
      if (not json.is_array())
        throw std::invalid_argument("Expected array type but we get :" +
                                    std::string(json.type_name()));
      if (json.size() != complete_sigs.size())
        throw std::invalid_argument(
            "Expected array size " + std::to_string(complete_sigs.size()) +
            " but we get :" + std::to_string(json.size()));
      for (size_t i = 0; i < complete_sigs.size(); ++i)
        FillMethodSig(method_call, json[i], complete_sigs[i]);
    } else if (complete_sigs.size() == 1) {
      const std::string current_sig = complete_sigs.front();
      switch (current_sig.front()) {
        case 'b':  // boolean
          if (json.is_boolean()) {
            std::cout << "append " << std::string(typeid(bool).name()) << " "
                      << json.dump() << std::endl;
            method_call << json.get<bool>();
          } else
            throw std::invalid_argument("Expected bool type but we get :" +
                                        std::string(json.type_name()));
          break;
        case 'y':  // byte
          AppendIntergerFromJson<uint8_t>(method_call, json);
          break;
        case 'n':  // int16
          AppendIntergerFromJson<int16_t>(method_call, json);
          break;
        case 'q':  // uint16
          AppendIntergerFromJson<uint16_t>(method_call, json);
          break;
        case 'i':  // int32
          AppendIntergerFromJson<int32_t>(method_call, json);
          break;
        case 'u':  // uint32
          AppendIntergerFromJson<uint32_t>(method_call, json);
          break;
        case 'x':  // int64
          AppendIntergerFromJson<int64_t>(method_call, json);
          break;
        case 't':  // uint64
          AppendIntergerFromJson<uint64_t>(method_call, json);
          break;
        case 'd':  // double
          if (json.is_number()) {
            std::cout << "append " << std::string(typeid(double).name()) << " "
                      << json.dump() << std::endl;
            method_call << json.get<double>();
          } else
            throw std::invalid_argument("Expected float type but we get :" +
                                        std::string(json.type_name()));
          break;
        case 's':  // string
          if (json.is_string()) {
            std::cout << "append " << std::string(typeid(std::string).name())
                      << " " << json.dump() << std::endl;
            method_call << json.get<std::string>();
          } else
            throw std::invalid_argument("Expected float type but we get :" +
                                        std::string(json.type_name()));
          break;
        case 'v':  // variant
          // TODO: extract data from json and fill to method_call
          break;
        case 'a':  // array, dict
          if (json.is_array() || json.is_object()) {
            std::string array_sig = current_sig.substr(1);
            std::string element_sig = array_sig.substr(1, array_sig.size() - 2);
            std::cout << "open container: " << array_sig << std::endl;
            method_call.openContainer(array_sig.c_str());
            std::cout << "container opened" << std::endl;

            if (current_sig[1] == '{')  // dict
              for (const auto& [key, value] : json.items()) {
                std::cout << "open dict entry " << element_sig << std::endl;
                method_call.openDictEntry(element_sig.c_str());

                std::cout << "dict opened" << std::endl;
                std::cout << "extract " << value.dump() << " from "
                          << json.dump() << std::endl;
                FillMethodSig(method_call, key, value, element_sig);
                std::cout << "close dict entry " << element_sig << std::endl;
                method_call.closeDictEntry();
                std::cout << "dict closed" << std::endl;
              }
            else if (current_sig[1] == '(')  // array
              for (const auto& j : json)
                FillMethodSig(method_call, j, array_sig);
            else
              throw std::invalid_argument("Invalid array signature: " +
                                          current_sig);
            std::cout << "close container: " << array_sig << std::endl;
            method_call.closeContainer();
          } else {
            throw std::invalid_argument("Expected array type but we get :" +
                                        std::string(json.type_name()));
          }

          break;
        case '(':  // struct
          FillMethodSig(method_call, json,
                        current_sig.substr(1, current_sig.size() - 2));
          break;
        default:;
      }
    }
  }

  template <typename T>
  void AppendIntergerFromJson(sdbus::MethodCall& method_call,
                              const nlohmann::json& json) {
    if (json.is_number_integer()) {
      std::cout << "typed append " << std::string(typeid(T).name()) << " "
                << json.dump() << std::endl;
      method_call << json.get<T>();
    } else
      throw std::invalid_argument(
          "Expected " + std::string(typeid(T).name()) +
          " type but we get :" + std::string(json.type_name()));
  }

  template <typename T>
  T JsonToSigType(const nlohmann::json& value, const std::string& sig) {
    if (sig == "s") return value.get<std::string>();
    if (sig == "i") return value.get<int32_t>();
    if (sig == "b") return value.get<bool>();
    if (sig == "d") return value.get<double>();
    throw std::invalid_argument("Unsupported signature: " + sig);
  }

  std::vector<std::string> split_signature(const std::string& sig) {
    std::string remain_sig = sig;
    std::vector<std::string> result;
    std::string::size_type pos;
    while (not remain_sig.empty()) {
      switch (remain_sig.front()) {
        case 'y':  // byte
        case 'b':  // boolean
        case 'n':  // int16
        case 'q':  // uint16
        case 'i':  // int32
        case 'u':  // uint32
        case 'x':  // int64
        case 't':  // uint64
        case 'd':  // double
        case 's':  // string
        case 'v':  // variant
          result.emplace_back(remain_sig.substr(0, 1));
          remain_sig = remain_sig.substr(1);
          break;
        case '(':  // struct
          pos = find_match_bracket(remain_sig, 0, '(', ')');
          result.emplace_back(remain_sig.substr(0, pos));
          remain_sig = remain_sig.substr(pos);
          break;

        case 'a':  // array
          char left;
          char right;
          if (remain_sig[1] == '(') {
            left = '(';
            right = ')';
          } else if (remain_sig[1] == '{') {
            left = '{';
            right = '}';
          }
          pos = find_match_bracket(remain_sig, 1, left, right);
          result.emplace_back(remain_sig.substr(0, pos));
          remain_sig = remain_sig.substr(pos);
          break;

        case '\0':  // invalid
        case 'h':   // unix file descriptor
        case 'r':   // general concept of struct
        case 'e':   // general concept of dict
        case 'o':   // name of object
        case 'g':   // signature
        case 'm':   // reserved
        case '*':   // reserved
        case '?':   // reserved
        case '@':   // reserved
        case '&':   // reserved
        case '^':   // reserved
          throw std::invalid_argument("Unsupported signature: " +
                                      remain_sig.substr(0, 1));
        default:
          throw std::invalid_argument("Unknown signature: " +
                                      remain_sig.substr(0, 1));
      }
    }
    return result;
  }
  static std::string::size_type find_match_bracket(
      const std::string& str, const std::string::size_type left_index,
      const char left, const char right) {
    int count = 1;
    for (std::string::size_type i = left_index + 1; i <= str.length(); i++) {
      if (count == 0) return i;
      if (str[i] == left)
        count++;
      else if (str[i] == right)
        count--;
    }
    return 0;
  }
};

}  // namespace dbus2http
