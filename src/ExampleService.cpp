//
// Created by yukunlin on 11/22/25.
//
#include <dbus2http/ExampleService.h>

#include <iostream>

namespace dbus2http {

ExampleService::ExampleService(sdbus::IConnection& connection)
    : AdaptorInterfaces(connection, sdbus::ObjectPath(kExamleObjectPath)) {
  registerAdaptor();
  getObject()
      .addVTable(sdbus::registerMethod("Method0").implementedAs(
          [] { return Method0(); }))
      .forInterface(kExampleInterfaceName);
  getObject()
      .addVTable(sdbus::registerMethod("Method1")
                     .withInputParamNames("age")
                     .withOutputParamNames("valid")
                     .implementedAs([](int32_t age) { return Method1(age); }))
      .forInterface(kExampleInterfaceName);
  getObject()
      .addVTable(
          sdbus::registerMethod("Method2")
              .withInputParamNames("num", "name2age", "name", "valid",
                                   "name2valid")
              .withOutputParamNames("age2valid")
              .implementedAs(
                  [](const std::tuple<int32_t, std::map<std::string, int32_t>>&
                         arg1,
                     const std::tuple<std::string, bool,
                                      std::map<std::string, bool>>& arg2) {
                    return Method2(arg1, arg2);
                  }))
      .forInterface(kExampleInterfaceName);
  getObject()
      .addVTable(sdbus::registerMethod("Method3")
                     .withInputParamNames("config")
                     .withOutputParamNames("status")
                     .implementedAs([](const sdbus::Variant& config) {
                       return Method3(config);
                     }))
      .forInterface(kExampleInterfaceName);
  getObject()
      .addVTable(sdbus::registerSignal("kExampleSignalName")
                     .withParameters<std::tuple<int32_t, std::string>>(
                         {"age", "name"}))
      .forInterface(kExampleInterfaceName);
}

void ExampleService::Method0() { std::cout << "Method0" << std::endl; }

bool ExampleService::Method1(int32_t age) {
  std::cout << "Method1:" << std::endl;
  std::cout << "  age: " << age << std::endl;
  return true;
}

std::map<int32_t, bool> ExampleService::Method2(
    const std::tuple<int32_t, std::map<std::string, int32_t>>& arg1,
    const std::tuple<std::string, bool, std::map<std::string, bool>>& arg2) {
  std::cout << "Method2:" << std::endl;

  std::cout << "  arg1: " << std::endl;
  std::cout << "    i: " << std::get<0>(arg1) << std::endl;
  std::cout << "    a{si}: " << std::endl;
  for (const auto& [k, v] : std::get<1>(arg1))
    std::cout << "      " << k << " -> " << v << std::endl;

  std::cout << "  arg2: " << std::endl;
  std::cout << "    s: " << std::get<0>(arg2) << std::endl;
  std::cout << "    b: " << (std::get<1>(arg2) ? "true" : "false") << std::endl;
  std::cout << "    a{sb}: " << std::endl;
  for (const auto& [k, v] : std::get<2>(arg2))
    std::cout << "      " << k << " -> " << (v ? "true" : "false") << std::endl;

  return {{3, true}, {4, false}};
}

bool ExampleService::Method3(const sdbus::Variant& config) {
  std::cout << "Method3:" << std::endl;

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

}  // namespace dbus2http