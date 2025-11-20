//
// Created by yukunlin on 2025/11/18.
//
#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <map>
#include <string>
#include <tuple>

const std::string kInterfaceName = "com.example.InterfaceName";
const std::string kServiceName = "com.example.ServiceName";
const std::string kObjectPath = "/path/to/object";

class ExampleService
    : public sdbus::AdaptorInterfaces<sdbus::Properties_adaptor> {
 public:
  explicit ExampleService(sdbus::IConnection& connection)
      : AdaptorInterfaces(connection, sdbus::ObjectPath(kObjectPath)) {
    registerAdaptor();
    getObject()
        .addVTable(sdbus::registerMethod("Method0").implementedAs(
            [this]() { return this->Method0(); }))
        .forInterface(kInterfaceName);
    getObject()
        .addVTable(sdbus::registerMethod("Method1")
                       .withInputParamNames("age")
                       .withOutputParamNames("valid")
                       .implementedAs(
                           [this](int32_t age) { return this->Method1(age); }))
        .forInterface(kInterfaceName);
    getObject()
        .addVTable(
            sdbus::registerMethod("Method2")
                .withInputParamNames("num", "name2age", "name", "valid",
                                     "name2valid")
                .withOutputParamNames("age2valid")
                .implementedAs(
                    [this](
                        const std::tuple<int32_t,
                                         std::map<std::string, int32_t>>& arg1,
                        const std::tuple<std::string, bool,
                                         std::map<std::string, bool>>& arg2) {
                      return this->Method2(arg1, arg2);
                    }))
        .forInterface(kInterfaceName);
    getObject()
        .addVTable(sdbus::registerMethod("Method3")
                       .withInputParamNames("config")
                       .withOutputParamNames("status")
                       .implementedAs([this](const sdbus::Variant& config) {
                         return this->Method3(config);
                       }))
        .forInterface(kInterfaceName);
  }

 private:
  // dbus-send --session --print-reply --type=method_call --dest=com.example.ServiceName /path/to/object com.example.InterfaceName.Method0
  void Method0() { std::cout << "Method0" << std::endl; }

  // dbus-send --session --print-reply --type=method_call --dest=com.example.ServiceName /path/to/object com.example.InterfaceName.Method1 int32:1
  bool Method1(int32_t age) {
    std::cout << "Method1：" << std::endl;
    std::cout << "  age: " << age << std::endl;
    return true;
  }

  // dbus-send --session --print-reply --type=method_call \
  // --dest=com.example.ServiceName /path/to/object com.example.InterfaceName.Method2 \
  // int32:1 dict:string:int32:hello,32 string:def boolean:false dict:string:boolean:world,true
  std::map<int32_t, bool> Method2(
      const std::tuple<int32_t, std::map<std::string, int32_t>>& arg1,
      const std::tuple<std::string, bool, std::map<std::string, bool>>& arg2) {
    std::cout << "Method2：" << std::endl;

    std::cout << "  arg1: " << std::endl;
    std::cout << "    i: " << std::get<0>(arg1) << std::endl;
    std::cout << "    a{si}: " << std::endl;
    for (const auto& [k, v] : std::get<1>(arg1))
      std::cout << "      " << k << " → " << v << std::endl;

    std::cout << "  arg1: " << std::endl;
    std::cout << "    s: " << std::get<0>(arg2) << std::endl;
    std::cout << "    b: " << std::get<1>(arg2) << std::endl;
    std::cout << "    a{sb}: " << std::endl;
    for (const auto& [k, v] : std::get<2>(arg2))
      std::cout << "      " << k << " → " << (v ? "true" : "false") << std::endl;

    return {{3, true}, {4, false}};
  }

  // dbus-send --print-reply --dest=com.example.ServiceName /path/to/object com.example.InterfaceName.Method3 variant:int32:4
  bool Method3(const sdbus::Variant& config) {
    std::cout << "Method3：" << std::endl;

    if (config.containsValueOfType<int32_t>()) {
      std::cout << config.get<int32_t>() << " (int32)" << std::endl;
    } else if (config.containsValueOfType<std::string>()) {
      std::cout << config.get<std::string>() << " (string)" << std::endl;
    } else if (config.containsValueOfType<bool>()) {
      std::cout << (config.get<bool>() ? "true" : "false") << " (boolean)"
                << std::endl;
    } else if (config.containsValueOfType<std::vector<int32_t>>()) {
      auto arr = config.get<std::vector<int32_t>>();
      std::cout << "[";
      for (auto num : arr) std::cout << num << ",";
      std::cout << "] (array<int32>)" << std::endl;
    } else {
      std::cout << "Unsupported type" << std::endl;
    }
    return true;
  }
};

int main() {
  try {
    const auto connection =
        sdbus::createSessionBusConnection(sdbus::ServiceName(kServiceName));

    ExampleService service(*connection);
    std::cout << "service launched" << std::endl;

    connection->enterEventLoop();
  } catch (const sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}