//
// Created by yukunlin on 2025/11/18.
//
#pragma once
#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <string>
#include <tuple>

namespace dbus2http {

const std::string kExampleServiceName = "com.example.ServiceName";
const std::string kExampleInterfaceName = "com.example.InterfaceName";
const std::string kExamleObjectPath = "/path/to/object";
const std::string kExampleSignalName = "Signal0";

class ExampleService
    : public sdbus::AdaptorInterfaces<sdbus::Properties_adaptor> {
  std::thread signal_thread_;
  bool stop_ = false;

 public:
  explicit ExampleService(sdbus::IConnection& connection);

  void start() {
    signal_thread_ = std::thread([&]() {
      int32_t i = 0;
      while (not stop_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        getObject()
            .emitSignal("kExampleSignalName")
            .onInterface(kExampleInterfaceName)
            .withArguments(i++, "hello world");
      }
    });
  }

  void stop() { stop_ = true; }

 private:
  // dbus-send --session --print-reply --type=method_call
  // --dest=com.example.ServiceName /path/to/object
  // com.example.InterfaceName.Method0
  static void Method0();

  // dbus-send --session --print-reply --type=method_call
  // --dest=com.example.ServiceName /path/to/object
  // com.example.InterfaceName.Method1 int32:1
  static bool Method1(int32_t age);

  // dbus-send --session --print-reply --type=method_call \
  // --dest=com.example.ServiceName /path/to/object com.example.InterfaceName.Method2 \
  // int32:1 dict:string:int32:hello,32 string:def boolean:false dict:string:boolean:world,true
  static std::map<int32_t, bool> Method2(
      const std::tuple<int32_t, std::map<std::string, int32_t>>& arg1,
      const std::tuple<std::string, bool, std::map<std::string, bool>>& arg2);

  // dbus-send --print-reply --dest=com.example.ServiceName /path/to/object
  // com.example.InterfaceName.Method3 variant:int32:4
  static bool Method3(const sdbus::Variant& config);
};

}  // namespace dbus2http