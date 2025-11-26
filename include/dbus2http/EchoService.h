//
// Created by yukunlin on 11/23/25.
//

#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <iostream>
#include <string>

#include "ExampleService.h"

namespace dbus2http {

const std::string kEchoServiceName = "com.test.ServiceName";
const std::string kEchoInterfaceName = "com.test.InterfaceName";
const std::string kEchoObjectPath = "/path/to/object";

class EchoService : public sdbus::AdaptorInterfaces<> {
 public:
  explicit EchoService(sdbus::IConnection& connection)
      : AdaptorInterfaces(connection, sdbus::ObjectPath(kEchoObjectPath)) {
    getObject()
        .addVTable(sdbus::registerMethod("method").implementedAs([] {}))
        .forInterface(kEchoInterfaceName);

    AddMethod("method_b", "b", {"arg0"});  // boolean
    AddMethod("method_y", "y", {"arg0"});  // byte
    AddMethod("method_n", "n", {"arg0"});  // int16
    AddMethod("method_q", "q", {"arg0"});  // uint16
    AddMethod("method_i", "i", {"arg0"});  // int32
    AddMethod("method_u", "u", {"arg0"});  // uint32
    AddMethod("method_x", "x", {"arg0"});  // int64
    AddMethod("method_t", "t", {"arg0"});  // uint64
    AddMethod("method_d", "d", {"arg0"});  // double
    AddMethod("method_s", "s", {"arg0"});  // string
    AddMethod("method_SisS", "(is)", {"arg0"});
    AddMethod("method_Sbynqiuxtds", "(bynqiuxtds)", {"arg0"});
    AddMethod("method_ai", "ai", {"arg0"});
    AddMethod("method_aSisS", "a(is)", {"arg0"});
    AddMethod("method_aSbynqiuxtdsS", "a(bynqiuxtds)", {"arg0"});
    AddMethod("method_aDssD", "a{ss}", {"arg0"});
    AddMethod("method_aDiiD", "a{ii}", {"arg0"});
    AddMethod("method_isaDsiD", "isa{si}", {"arg0", "arg1", "arg2"});
    AddMethod("method_iaDiSssaSiiaDssDSSD", "ia{i(ssa(iia{ss}))}",
              {"arg0", "arg1"});
    AddMethod("method_v", "v", {"arg0"});
    AddMethod("method_iv", "iv", {"arg0", "arg1"});
    AddMethod("method_aDsvD", "a{sv}", {"arg0"});

    AddSignal("signal_is", "is", {"arg0", "arg1"});
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
      PLOGI << "method: " << method_name;
    };
    getObject().addVTable(method_item).forInterface(kEchoInterfaceName);
  }
  void AddSignal(const std::string& signal_name,
                 const std::string& signature,
                 const std::vector<std::string>& param_names) {
    sdbus::SignalVTableItem signal_item;
    signal_item.name = signal_name;
    signal_item.signature = signature;
    signal_item.paramNames = param_names;
    getObject().addVTable(signal_item).forInterface(kEchoInterfaceName);
  }
};

}  // namespace dbus2http