//
// Created by yukunlin on 11/22/25.
//

#include <sdbus-c++/sdbus-c++.h>

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

namespace dbus2http {



static int add(int a, int b) { return a + b; }
uint64_t fibonacci(uint64_t number) {
  return number < 2 ? number : fibonacci(number - 1) + fibonacci(number - 2);
}

// 传统 TDD 风格
TEST_CASE("call method i", "[i][i]") {
  const auto conn = sdbus::createSessionBusConnection(
      sdbus::ServiceName(dbus2http::kServiceName));
  std::thread dbus_thread([&conn] {
    try {
      EchoService service(*conn);

      conn->enterEventLoop();
    } catch (const sdbus::Error& e) {
      std::cerr << e.what() << std::endl;
    };
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  SECTION("Positive numbers") {
    std::cout << "case 1" << std::endl;
    REQUIRE(add(2, 3) == 5);
    CHECK(add(10, 20) == 30);
  }

  SECTION("Negative numbers") {
    std::cout << "case 2" << std::endl;
    REQUIRE(add(-2, -3) == -5);
    REQUIRE(add(-10, 5) == -5);
  }

  SECTION("Boundary values") {
    std::cout << "case 3" << std::endl;
    REQUIRE(add(INT_MAX, 0) == INT_MAX);
    REQUIRE(add(INT_MIN, 0) == INT_MIN);
  }
  conn->leaveEventLoop();
  dbus_thread.join();
}
}  // namespace dbus2http
