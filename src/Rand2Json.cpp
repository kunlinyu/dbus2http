//
// Created by yukunlin on 2025/11/27.
//

#include <dbus2http/Rand2Json.h>
nlohmann::json dbus2http::Rand2Json::RandJson(
    const std::vector<Argument>& args) {
  nlohmann::json result;
  int i = 0;
  for (const auto& arg : args) {
    std::string arg_name = arg.name;
    if (arg_name.empty()) arg_name = "arg" + std::to_string(i++);
    result[arg_name] = RandJson(arg.type);
  }
  return result;
}
nlohmann::json dbus2http::Rand2Json::RandVariant() {
  std::string element_sig = "a{si}";  // TODO random select a signature

  PLOGD << "variant element signature " << element_sig;
  PLOGD << "open variant " << element_sig;
  nlohmann::json value = RandJson(element_sig);
  PLOGD << "exit variant " << element_sig;

  nlohmann::json result = {{"variant", std::string(element_sig)},
                           {"value", value}};

  return result;
}
nlohmann::json dbus2http::Rand2Json::RandJson(const std::string& sig) {
  PLOGD << "RandJson sig: " << sig;
  std::vector<std::string> complete_sigs = SignatureUtils::split(sig);
  if (complete_sigs.size() > 1) {
    nlohmann::json result = nlohmann::json::array();
    for (const auto& s : complete_sigs) result.push_back(RandJson(s));
    return result;
  }

  switch (sig.front()) {
    case 'b':  // boolean
      bool b;
      PLOGD << "extract bool";
      // message >> b;
      return b;
    case 'y':  // byte
      return dist_pos_(gen);
    case 'n':  // int16
      return dist_neg_(gen);
    case 'q':  // uint16
      return dist_pos_(gen);
    case 'i':  // int32
      return dist_neg_(gen);
    case 'u':  // uint32
      return dist_pos_(gen);
    case 'x':  // int64
      return dist_neg_(gen);
    case 't':  // uint64
      return dist_pos_(gen);
    case 'd':  // double
      return dist_neg_(gen);
    case 's':  // string
      return "hello";
    case 'v':  // variant
      return RandVariant();
    case '(':  // struct
    {
      std::string element_sig = sig.substr(1, sig.size() - 2);
      auto j = RandJson(element_sig);
      return j;
    }
    case 'a':               // array, dict
      if (sig[1] == '{') {  // dict
        nlohmann::json result;
        std::string array_sig = sig.substr(1);
        std::string element_sig = array_sig.substr(1, array_sig.size() - 2);
        for (size_t i = 0; i < dist_repeat_(gen); ++i) {
          nlohmann::json key;
          key = RandJson(element_sig.substr(0, 1));
          std::string key_str;
          if (key.is_boolean())
            key_str = std::to_string(key.get<bool>());
          else if (key.is_number_integer())
            key_str = std::to_string(key.get<long long>());
          else if (key.is_number())
            key_str = std::to_string(key.get<double>());
          else if (key.is_string())
            key_str = key.get<std::string>();
          result[key_str] = RandJson(element_sig.substr(1));
        }
        return result;
      }
      if (sig[1] == '(') {  // array
        nlohmann::json result = nlohmann::json::array();
        std::string array_sig = sig.substr(1);
        std::string element_sig = array_sig.substr(1, array_sig.size() - 2);
        for (size_t i = 0; i < dist_repeat_(gen); ++i) {
          nlohmann::json value = RandJson(element_sig);
          result.push_back(value);
        }
        return result;
      }
      {  // single charactor array
        nlohmann::json result = nlohmann::json::array();
        std::string element_sig = sig.substr(1);
        for (size_t i = 0; i < dist_repeat_(gen); ++i) {
          nlohmann::json value = RandJson(element_sig);
          result.push_back(value);
        }
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