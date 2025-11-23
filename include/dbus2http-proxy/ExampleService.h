//
// Created by yukunlin on 2025/11/18.
//
#include <sdbus-c++/sdbus-c++.h>

#include <map>
#include <string>
#include <tuple>

namespace dbus2http {

const std::string kExampleServiceName = "com.example.ServiceName";
const std::string kExampleInterfaceName = "com.example.InterfaceName";
const std::string kExamleObjectPath = "/path/to/object";

class ExampleService
    : public sdbus::AdaptorInterfaces<sdbus::Properties_adaptor> {
 public:
  explicit ExampleService(sdbus::IConnection& connection);

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