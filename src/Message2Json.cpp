//
// Created by yukunlin on 2025/11/25.
//

#include <dbus2http/Message2Json.h>
#include <dbus2http/SignatureUtils.h>
#include <plog/Log.h>

namespace dbus2http {

nlohmann::json Message2Json::ExtractMessage(sdbus::Message& message,
                                            const std::vector<Argument>& args) {
  nlohmann::json result;
  int i = 0;
  for (const auto& arg : args) {
    std::string arg_name = arg.name;
    if (arg_name.empty()) arg_name = "arg" + std::to_string(i++);
    result[arg_name] = ExtractMessage(message, arg.type);
  }
  return result;
}

nlohmann::json Message2Json::ExtractVariant(sdbus::Message& message) {
  PLOGD << "Extract Variant";
  const auto [sig, element_sig] = message.peekType();
  if (sig != 'v')
    throw std::invalid_argument("Expected variant type but got " +
                                std::string(1, sig));
  if (element_sig == nullptr)
    throw std::invalid_argument("Variant has NULL element type");
  PLOGD << "variant element signature " << element_sig;
  PLOGD << "open variant " << element_sig;
  message.enterVariant(element_sig);
  nlohmann::json value = ExtractMessage(message, element_sig);
  PLOGD << "exit variant " << element_sig;
  message.exitVariant();

  nlohmann::json result = {{"variant", std::string(element_sig)},
                           {"value", value}};

  return result;
}

nlohmann::json Message2Json::ExtractMessage(sdbus::Message& message,
                                            const std::string& sig) {
  PLOGD << "ExtractMessage sig: " << sig;
  std::vector<std::string> complete_sigs = SignatureUtils::split(sig);
  if (complete_sigs.size() > 1) {
    nlohmann::json result = nlohmann::json::array();
    for (const auto& s : complete_sigs)
      result.push_back(ExtractMessage(message, s));
    return result;
  }

  switch (sig.front()) {
    case 'b':  // boolean
      bool b;
      PLOGD << "extract bool";
      message >> b;
      return b;
    case 'y':  // byte
      return get_int<uint8_t>(message);
    case 'n':  // int16
      return get_int<int16_t>(message);
    case 'q':  // uint16
      return get_int<uint16_t>(message);
    case 'i':  // int32
      PLOGD << "extract int32";
      return get_int<int32_t>(message);
    case 'u':  // uint32
      PLOGD << "extract uint32";
      return get_int<uint32_t>(message);
    case 'x':  // int64
      return get_int<int64_t>(message);
    case 't':  // uint64
      return get_int<uint64_t>(message);
    case 'd':  // double
      return get_int<double>(message);
    case 's':  // string
      PLOGD << "extract string";
      return get_int<std::string>(message);
    case 'v':  // variant
      return ExtractVariant(message);
    case '(':  // struct
    {
      std::string element_sig = sig.substr(1, sig.size() - 2);
      message.enterStruct(element_sig.c_str());
      auto j = ExtractMessage(message, element_sig);
      message.exitStruct();
      return j;
    }
    case 'a':               // array, dict
      if (sig[1] == '{') {  // dict
        nlohmann::json result;
        std::string array_sig = sig.substr(1);
        std::string element_sig = array_sig.substr(1, array_sig.size() - 2);
        PLOGD << "enter container " << array_sig;
        message.enterContainer(array_sig.c_str());
        PLOGD << "entered container";
        while (message.enterDictEntry(element_sig.c_str())) {
          PLOGD << "entered dict " << element_sig;
          nlohmann::json key;
          PLOGD << "extract key";
          key = ExtractMessage(message, element_sig.substr(0, 1));
          std::string key_str;
          if (key.is_boolean())
            key_str = std::to_string(key.get<bool>());
          else if (key.is_number_integer())
            key_str = std::to_string(key.get<long long>());
          else if (key.is_number())
            key_str = std::to_string(key.get<double>());
          else if (key.is_string())
            key_str = key.get<std::string>();
          PLOGD << "extracted key: " << key_str;
          result[key_str] = ExtractMessage(message, element_sig.substr(1));
          PLOGD << "extracted value" << result[key_str].dump();
          message.exitDictEntry();
          PLOGD << "exited dict entry";
        }
        message.clearFlags();
        message.exitContainer();
        return result;
      }
      if (sig[1] == '(') {  // array
        nlohmann::json result = nlohmann::json::array();
        std::string array_sig = sig.substr(1);
        std::string element_sig = array_sig.substr(1, array_sig.size() - 2);
        PLOGD << "enter container " << array_sig;
        message.enterContainer(array_sig.c_str());
        PLOGD << "entered container " << array_sig;
        while (true) {
          PLOGD << "enter struct " << element_sig;
          if (not message.enterStruct(element_sig.c_str())) break;
          PLOGD << "entered struct " << element_sig;
          nlohmann::json value = ExtractMessage(message, element_sig);
          if (message) {
            result.push_back(value);
            PLOGD << "exit struct " << element_sig;
            message.exitStruct();
            PLOGD << "exited struct " << element_sig;
          } else
            break;
        }
        message.clearFlags();
        message.exitContainer();
        return result;
      }
      if (sig == "ay" and config_.binary_support) {
        std::vector<uint8_t> binaries;
        PLOGD << "enter container y";
        message.enterContainer("y");
        while (true) {
          auto y = get_int<uint8_t>(message);
          if (message)
            binaries.push_back(y);
          else
            break;
        }
        message.clearFlags();
        PLOGD << "exit container y";
        message.exitContainer();
        return nlohmann::json::binary(binaries);
      }
      PLOGD << sig << " " << config_.binary_support;
      {  // single charactor array
        nlohmann::json result = nlohmann::json::array();
        std::string element_sig = sig.substr(1);
        PLOGD << "enter container " << element_sig;
        message.enterContainer(element_sig.c_str());
        while (true) {
          nlohmann::json value = ExtractMessage(message, element_sig);
          if (message)
            result.push_back(value);
          else
            break;
        }
        message.clearFlags();
        PLOGD << "exit container " << element_sig;
        message.exitContainer();
        return result;
      }
    case 'h':  // unix file descriptor
    {
      std::vector<uint8_t> binaries;
      sdbus::UnixFd fd;
      message >> fd;

      std::vector<uint8_t> buffer(4096);
      ssize_t bytes_read;
      while ((bytes_read = read(fd.get(), buffer.data(), buffer.size())) > 0) {
        binaries.insert(binaries.end(), buffer.begin(),
                        buffer.begin() + bytes_read);
        if (binaries.size() > config_.max_file_descriptor_size)
          throw std::runtime_error("File descriptor data exceeds 10MB limit");
      }
      if (bytes_read == -1) {
        if (errno == EINTR)
          throw std::runtime_error("read() interrupted by signal");
        throw std::runtime_error("read() failed: " +
                                 std::string(strerror(errno)));
      }
      return nlohmann::json::binary(binaries);
    }

    case '\0':  // invalid
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
nlohmann::json Message2Json::WrapHeader(sdbus::Message& message,
                                        const nlohmann::json& data) {
  nlohmann::json result;
  result["type"] = "signal";
  result["interface"] = json_null(message.getInterfaceName());
  result["member"] = json_null(message.getMemberName());
  result["sender"] = json_null(message.getSender());
  result["path"] = json_null(message.getPath());
  result["destination"] = json_null(message.getDestination());
  result["data"] = data;
  return result;
}
}  // namespace dbus2http