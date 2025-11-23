//
// Created by yukunlin on 11/23/25.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <string>
#include <iostream>

namespace dbus2http {

const std::string kEchoServiceName = "com.test.ServiceName";
const std::string kEchoInterfaceName = "com.test.InterfaceName";
const std::string kEchoObjectPath = "/path/to/object";

class test_methods : public sdbus::AdaptorInterfaces<> {

 public:
  explicit test_methods(sdbus::IConnection& connection)
      : AdaptorInterfaces(connection, sdbus::ObjectPath(kEchoObjectPath)) {
    getObject()
        .addVTable(sdbus::registerMethod("method").implementedAs([] {}))
        .forInterface(kEchoInterfaceName);
    AddMethod("method_i", "i", {"arg0"});
    AddMethod("method_y", "y", {"arg0"});
    AddMethod("method_b", "b", {"arg0"});
    AddMethod("method_isaDsiD", "isa{si}", {"arg0", "arg1", "arg2"});
  }
  void AddMethod(const std::string& method_name,
                 const std::string& signature_in_out,
                 const std::vector<std::string>& param_names) {
    sdbus::MethodVTableItem method_item;
    method_item.name = method_name;
    method_item.inputSignature = signature_in_out;
    method_item.outputSignature = signature_in_out;
    method_item.withInputParamNames(param_names);
    method_item.withOutputParamNames(param_names);
    method_item.callbackHandler = [method_name](sdbus::MethodCall method_call) {
      sdbus::MethodReply method_reply = method_call.createReply();
      method_call.copyTo(method_reply, true);
      method_reply.send();
      std::cout << "method: " << method_name << std::endl;
    };
    getObject().addVTable(method_item).forInterface(kEchoInterfaceName);
  }
};

}  // namespace dbus2http