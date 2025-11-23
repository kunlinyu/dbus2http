//
// Created by yukunlin on 2025/11/20.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <nlohmann/json.hpp>

#include "SignatureUtils.h"

namespace dbus2http {

class Message2Json {
 public:
  static nlohmann::json ExtractMethod(sdbus::MethodReply& method_reply,
                                      const Method& method_type) {
    nlohmann::json result;
    int i = 0;
    for (const auto& arg : method_type.args) {
      if (arg.direction != "out") continue;
      std::string arg_name = arg.name;
      if (arg_name.empty()) arg_name = "arg" + std::to_string(i++);
      result[arg_name] = ExtractMethod(method_reply, arg.type);
    }
    return result;
  }

  template <typename T>
  static T get_int(sdbus::MethodReply& method_reply) {
    T result;
    method_reply >> result;
    return result;
  }

  static nlohmann::json ExtractMethod(sdbus::MethodReply& method_reply,
                                      const std::string& sig) {
    std::vector<std::string> complete_sigs = SignatureUtils::split(sig);
    if (complete_sigs.size() > 1) {
      nlohmann::json result = nlohmann::json::array();
      for (const auto& s : complete_sigs)
        result.push_back(ExtractMethod(method_reply, s));
      return result;
    }

    switch (sig.front()) {
      case 'b':  // boolean
        bool b;
        std::cout << "extract bool" << std::endl;
        method_reply >> b;
        return b;
      case 'y':  // byte
        return get_int<uint8_t>(method_reply);
      case 'n':  // int16
        return get_int<int16_t>(method_reply);
      case 'q':  // uint16
        return get_int<uint16_t>(method_reply);
      case 'i':  // int32
        std::cout << "extract int32" << std::endl;
        return get_int<int32_t>(method_reply);
      case 'u':  // uint32
        std::cout << "extract uint32" << std::endl;
        return get_int<uint32_t>(method_reply);
      case 'x':  // int64
        return get_int<int64_t>(method_reply);
      case 't':  // uint64
        return get_int<uint64_t>(method_reply);
      case 'd':  // double
        return get_int<double>(method_reply);
      case 's':  // string
        return get_int<std::string>(method_reply);
      case 'v':  // variant
        // TODO: support variant
        // sdbus::Variant v;
        // method_reply >> v;
        return {};
      case '(':  // struct
      {
        std::string element_sig = sig.substr(1, sig.size() - 2);
        method_reply.enterStruct(element_sig.c_str());
        auto j = ExtractMethod(method_reply, element_sig);
        method_reply.exitStruct();
        return j;
      }
      case 'a':               // array, dict
        if (sig[1] == '{') {  // dict
          nlohmann::json result;
          std::string array_sig = sig.substr(1);
          std::string element_sig = array_sig.substr(1, array_sig.size() - 2);
          std::cout << "enter container " << array_sig << std::endl;
          method_reply.enterContainer(array_sig.c_str());
          std::cout << "entered container" << std::endl;
          while (method_reply.enterDictEntry(element_sig.c_str())) {
            std::cout << "entered dict " << element_sig << std::endl;
            nlohmann::json key;
            std::cout << "extract key" << std::endl;
            key = ExtractMethod(method_reply, element_sig.substr(0, 1));
            std::string key_str;
            if (key.is_boolean())
              key_str = std::to_string(key.get<bool>());
            else if (key.is_number_integer())
              key_str = std::to_string(key.get<long long>());
            else if (key.is_number())
              key_str = std::to_string(key.get<double>());
            else if (key.is_string())
              key_str = key.get<std::string>();
            std::cout << "extracted key: " << key_str << std::endl;
            result[key_str] =
                ExtractMethod(method_reply, element_sig.substr(1));
            std::cout << "extracted value" << result[key_str].dump()
                      << std::endl;
            method_reply.exitDictEntry();
            std::cout << "exited dict entry" << std::endl;
          }
          method_reply.clearFlags();
          method_reply.exitContainer();
          return result;
        }
        if (sig[1] == '(') {  // array
          nlohmann::json result = nlohmann::json::array();
          std::string array_sig = sig.substr(1);
          std::string element_sig = array_sig.substr(1, array_sig.size() - 2);
          std::cout << "enter container " << array_sig << std::endl;
          method_reply.enterContainer(array_sig.c_str());
          while (true) {
            nlohmann::json value = ExtractMethod(method_reply, element_sig.substr(1));
            if (method_reply)
              result.push_back(value);
            else
              break;
          }
          method_reply.clearFlags();
          method_reply.exitContainer();
          return result;
        }
        {  // single charactor array
          nlohmann::json result = nlohmann::json::array();
          std::string element_sig = sig.substr(1);
          std::cout << "enter container " << element_sig << std::endl;
          method_reply.enterContainer(element_sig.c_str());
          while (true) {
            nlohmann::json value = ExtractMethod(method_reply, element_sig);
            if (method_reply)
              result.push_back(value);
            else
              break;
          }
          method_reply.clearFlags();
          std::cout << "exit container " << element_sig << std::endl;
          method_reply.exitContainer();
          return result;
        }

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
        throw std::invalid_argument("Unsupported signature: " + sig);
      default:
        throw std::invalid_argument("Unknown signature: " + sig);
    }
  }
};
};  // namespace dbus2http
