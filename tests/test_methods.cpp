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
  std::cout << "run one case" << std::endl;
  try {
    std::cout << "run post" << std::endl;
    for (int i = 0; i < 5; i++) {
      res = client.Post(
          "/dbus/com.test.ServiceName/path/to/object/"
          "com.test.InterfaceName." + method_name,
          request, {"Content-Type: application/json"});
      if (res) break;
      std::cout << "retry" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "post reply" << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "failed to send http request to echo service: " << e.what() << std::endl;
  }


  if (res) {
    std::cout << res->status << std::endl;
    if (res->status == 200)
      std::cout << res->body << std::endl;
    else
      std::cerr << res->status << std::endl;
  } else {
    std::cerr << "request failed: " << res.error() << std::endl;
  }

  REQUIRE(res);
  REQUIRE(res->status == 200);
  // REQUIRE(res->body == request);
}

TEST_CASE("call methods", "[i][i]") {
  const auto conn = sdbus::createSessionBusConnection(
      sdbus::ServiceName(dbus2http::kEchoServiceName));
  std::thread dbus_thread([&conn] {
    try {
      EchoService service(*conn);

      conn->enterEventLoop();
    } catch (const sdbus::Error& e) {
      std::cerr << "echo service launch faild: " << e.what() << std::endl;
    };
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  Dbus2Http dbus2http;
  try {
    dbus2http.start();
  } catch (const std::exception& e) {
    std::cerr << "start dbus2http failed: " << e.what() << std::endl;
  }

  std::cout << "new client" << std::endl;
  httplib::Client client("http://localhost:8080");
  SECTION("method b") {  // boolean
    run_one_case(client, "{\"arg0\":true}", "method_b");
  }
  SECTION("method y") {  // byte
    run_one_case(client, "{\"arg0\":255}", "method_y");
  }
  SECTION("method n") {  // int16
    run_one_case(client, "{\"arg0\":-32768}", "method_n");
  }
  SECTION("method q") {  // uint16
    run_one_case(client, "{\"arg0\":65535}", "method_q");
  }
  SECTION("method i") {  // int32
    run_one_case(client, "{\"arg0\":-2147483648}", "method_i");
  }
  SECTION("method u") {  // uint32
    run_one_case(client, "{\"arg0\":4294967295}", "method_u");
  }
  SECTION("method x") {  // int64
    run_one_case(client, "{\"arg0\":-9223372036854775808}", "method_x");
  }
  SECTION("method t") {  // uint64
    run_one_case(client, "{\"arg0\":18446744073709551615}", "method_t");
  }
  SECTION("method d") {  // double
    run_one_case(client, R"({"arg0":123.45})", "method_d");
  }
  SECTION("method s") {  // string
    run_one_case(client, R"({"arg0":"hello"})", "method_s");
  }
  SECTION("method (is)") {  // string
    run_one_case(client, R"({"arg0":[123,"hello"]})", "method_SisS");
  }
  SECTION("method isa{si}") {
    std::string request = R"({"arg0":123,"arg1":"hello","arg2":{"Alic":23,"Bob":45}})";
    run_one_case(client, request, "method_isaDsiD");
  }
  dbus2http.stop();
  conn->leaveEventLoop();
  dbus_thread.join();
}
}  // namespace dbus2http
