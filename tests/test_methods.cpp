//
// Created by yukunlin on 11/22/25.
//

#include <sdbus-c++/sdbus-c++.h>

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

#include "dbus2http-proxy/Dbus2Http.h"
#include "dbus2http-proxy/test_methods.h"

namespace dbus2http {

void run_one_case(httplib::Client& client, const std::string& request,
                  const std::string& method_name) {
  auto res = client.Post(
      "/dbus/com.test.ServiceName/path/to/object/"
      "com.test.InterfaceName." + method_name,
      request, {"Content-Type: application/json"});
  REQUIRE(res);
  REQUIRE(res->status == 200);
  REQUIRE(res->body == request);
}

TEST_CASE("call methods", "[i][i]") {
  const auto conn = sdbus::createSessionBusConnection(
      sdbus::ServiceName(dbus2http::kEchoServiceName));
  std::thread dbus_thread([&conn] {
    try {
      test_methods service(*conn);

      conn->enterEventLoop();
    } catch (const sdbus::Error& e) {
      std::cerr << e.what() << std::endl;
    };
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  Dbus2Http dbus2http;
  dbus2http.start();
  httplib::Client client("http://localhost:8080");

  SECTION("method i") {
    run_one_case(client, R"({"arg0":123})", "method_i");
  }
  SECTION("method b") {
    run_one_case(client, R"({"arg0":true})", "method_b");
  }
  SECTION("method y") {
    run_one_case(client, R"({"arg0":3})", "method_y");
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
