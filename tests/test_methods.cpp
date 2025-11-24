//
// Created by yukunlin on 11/22/25.
//

#include <sdbus-c++/sdbus-c++.h>

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

#include "dbus2http-proxy/Dbus2Http.h"
#include "dbus2http-proxy/EchoService.h"

namespace dbus2http {

void run_one_case(httplib::Client& client, const std::string& request,
                  const std::string& method_name) {
  httplib::Result res;
  PLOGI << "run one case";
  try {
    PLOGI << "run post";
    for (int i = 0; i < 5; i++) {
      res = client.Post(
          "/dbus/com.test.ServiceName/path/to/object/"
          "com.test.InterfaceName." + method_name,
          request, {"Content-Type: application/json"});
      if (res) break;
      PLOGI << "retry";
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    PLOGI << "post reply";
  } catch (const std::exception& e) {
    PLOGE << "failed to send http request to echo service: " << e.what();
  }


  if (res) {
    PLOGI << res->status;
    if (res->status == 200)
      PLOGI << res->body;
    else
      PLOGE << res->status;
  } else {
    PLOGE << "request failed: " << res.error();
  }

  REQUIRE(res);
  REQUIRE(res->status == 200);
  REQUIRE(res->body == request);
}

TEST_CASE("call methods", "[i][i]") {
  const auto conn = sdbus::createSessionBusConnection(
      sdbus::ServiceName(dbus2http::kEchoServiceName));
  std::thread dbus_thread([&conn] {
    try {
      EchoService service(*conn);

      conn->enterEventLoop();
    } catch (const sdbus::Error& e) {
      PLOGE << "echo service launch faild: " << e.what();
    };
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  Dbus2Http dbus2http({"com.test" }, false);
  try {
    dbus2http.start(8080);
  } catch (const std::exception& e) {
    PLOGE << "start dbus2http failed: " << e.what();
  }

  PLOGI << "new client";
  httplib::Client client("http://localhost:8080");
  SECTION("method b") {  // boolean
    run_one_case(client, "{\"arg0\":true}", "method_b");
    run_one_case(client, "{\"arg0\":false}", "method_b");
  }
  SECTION("method y") {  // byte
    run_one_case(client, "{\"arg0\":0}", "method_y");
    run_one_case(client, "{\"arg0\":255}", "method_y");
  }
  SECTION("method n") {  // int16
    run_one_case(client, "{\"arg0\":32767}", "method_n");
    run_one_case(client, "{\"arg0\":-32768}", "method_n");
  }
  SECTION("method q") {  // uint16
    run_one_case(client, "{\"arg0\":0}", "method_q");
    run_one_case(client, "{\"arg0\":65535}", "method_q");
  }
  SECTION("method i") {  // int32
    run_one_case(client, "{\"arg0\":2147483647}", "method_i");
    run_one_case(client, "{\"arg0\":-2147483648}", "method_i");
  }
  SECTION("method u") {  // uint32
    run_one_case(client, "{\"arg0\":0}", "method_u");
    run_one_case(client, "{\"arg0\":4294967295}", "method_u");
  }
  SECTION("method x") {  // int64
    run_one_case(client, "{\"arg0\":9223372036854775807}", "method_x");
    run_one_case(client, "{\"arg0\":-9223372036854775808}", "method_x");
  }
  SECTION("method t") {  // uint64
    run_one_case(client, "{\"arg0\":0}", "method_t");
    run_one_case(client, "{\"arg0\":18446744073709551615}", "method_t");
  }
  SECTION("method d") {  // double
    run_one_case(client, "{\"arg0\":-123.45}", "method_d");
    run_one_case(client, "{\"arg0\":123.45}", "method_d");
    run_one_case(client, "{\"arg0\":0.0}", "method_d");
  }
  SECTION("method s") {  // string
    run_one_case(client, "{\"arg0\":\"hello\"}", "method_s");
    run_one_case(client, "{\"arg0\":\"让我们说中文\"}", "method_s");
  }
  SECTION("method (is)") {
    run_one_case(client, "{\"arg0\":[123,\"hello\"]}", "method_SisS");
    run_one_case(client, "{\"arg0\":[-123,\"让我们说中文\"]}", "method_SisS");
  }
  SECTION("method (bynqiuxtds)") {
    std::string request;
    request = "{\"arg0\":[true,1,1,1,1,1,1,1,1.1,\"hello\"]}";
    run_one_case(client, request, "method_Sbynqiuxtds");
    request = "{\"arg0\":[false,0,0,0,0,0,0,0,0.1,\"让我们说中文\"]}";
    run_one_case(client, request, "method_Sbynqiuxtds");
  }
  SECTION("method ai") {
    run_one_case(client, "{\"arg0\":[1]}", "method_ai");
    run_one_case(client, "{\"arg0\":[1,2]}", "method_ai");
    run_one_case(client, "{\"arg0\":[1,2,3,4,5,6,7,8,9]}", "method_ai");
    run_one_case(client, "{\"arg0\":[]}", "method_ai");
  }
  SECTION("method a(is)") {
    run_one_case(client, "{\"arg0\":[[1,\"hello\"],[2,\"world\"]]}", "method_aSisS");
  }
  SECTION("method a(bynqiuxtds)") {
  }
  SECTION("method a{ss}") {
  }
  SECTION("method a{ii}") {
  }
  SECTION("method isa{si}") {
    std::string request = R"({"arg0":123,"arg1":"hello","arg2":{"Alic":23,"Bob":45}})";
    run_one_case(client, request, "method_isaDsiD");
  }
  SECTION("method ia{i(ssa(iia{ss}))}") {
  }
  dbus2http.stop();
  conn->leaveEventLoop();
  dbus_thread.join();
}
}  // namespace dbus2http
