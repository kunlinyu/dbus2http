//
// Created by yukunlin on 11/22/25.
//

#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <sdbus-c++/sdbus-c++.h>

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

#include "dbus2http/Dbus2Http.h"
#include "dbus2http/EchoService.h"
#include "dbus2http/FileLineFormatter.h"

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
          "com.test.InterfaceName." +
              method_name,
          request, {"Content-Type: application/json"},
          [&](size_t current, size_t total) {
            PLOGI << "upload progress: " << current << "/" << total;
            return true;
          });
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
    if (res->status == 200) {
      PLOGI << res->body;
    }
  } else {
    PLOGE << "request failed: " << res.error();
  }

  REQUIRE(res);
  REQUIRE(res->status == 200);
  nlohmann::json req_j = nlohmann::json::parse(request);
  REQUIRE(res->body == req_j.dump());
}

static plog::ConsoleAppender<FileLineFormatter> consoleAppender;
std::unique_ptr<sdbus::IConnection> conn;
std::thread dbus_thread;
std::unique_ptr<Dbus2Http> dbus2http;
bool first = true;
TEST_CASE("call methods", "[i][i]") {
  if (first) {
    plog::init(plog::debug, &consoleAppender);
    first = false;

    conn = sdbus::createSessionBusConnection(
        sdbus::ServiceName(dbus2http::kEchoServiceName));
    dbus_thread = std::thread([&] {
      try {
        EchoService service(*conn);

        conn->enterEventLoop();
      } catch (const sdbus::Error& e) {
        PLOGE << "echo service launch faild: " << e.what();
      };
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::vector<std::string> service_prefixes = {"com.test"};
    dbus2http = std::make_unique<Dbus2Http>(service_prefixes, false);
    try {
      dbus2http->start(8080);
    } catch (const std::exception& e) {
      PLOGE << "start dbus2http failed: " << e.what();
    }
  }

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
    run_one_case(client, "{\"arg0\":[[1,\"hello\"],[2,\"world\"]]}",
                 "method_aSisS");
  }
  SECTION("method a(bynqiuxtds)") {
    std::string request;
    request =
        "{\"arg0\":[[true,1,1,1,1,1,1,1,1.1,\"hello\"],[false,0,0,0,0,0,0,0,0."
        "1,\"让我们说中文\"]]}";
    run_one_case(client, request, "method_aSbynqiuxtdsS");
  }
  SECTION("method a{ss}") {
    run_one_case(client, "{\"arg0\":{\"key1\":\"hello\",\"key2\":\"world\"}}",
                 "method_aDssD");
    run_one_case(client, "{\"arg0\":{\"key_a\":\"你好\",\"key_b\":\"世界\"}}",
                 "method_aDssD");
  }
  SECTION("method a{ii}") {
    run_one_case(client, "{\"arg0\":{\"1\":2,\"3\":4}}", "method_aDiiD");
    run_one_case(client, "{\"arg0\":{\"-1\":-2,\"-3\":-4}}", "method_aDiiD");
  }
  SECTION("method isa{si}") {
    std::string request =
        R"({"arg0":123,"arg1":"hello","arg2":{"Alic":23,"Bob":45}})";
    run_one_case(client, request, "method_isaDsiD");
  }
  SECTION("method ia{i(ssa(iia{ss}))}") {
    std::string request = R"(
{
  "arg0":123,
  "arg1": {
    "0":["hello", "world", [ [1, 2, {"key": "value"}], [3, 4, {"你好": "世界"}] ] ],
    "1":["hello", "world", [ [1, 2, {"key": "value"}], [3, 4, {"你好": "世界"}] ] ]
  }
}
)";
    run_one_case(client, request, "method_iaDiSssaSiiaDssDSSD");
  }
  SECTION("method v") {
    run_one_case(client, "{\"arg0\":{\"variant\":\"i\",\"value\":123}}",
                 "method_v");
  }
  SECTION("method iv") {
    run_one_case(client,
                 "{\"arg0\":123,\"arg1\":{\"variant\":\"i\",\"value\":123}}",
                 "method_iv");
  }
  SECTION("method a{sv}") {
    std::string request = R"*(
{
  "arg0": {
    "key1": {
      "variant":"i",
      "value":123
    },
    "key2": {
      "variant":"(iis)",
      "value": [123, 456, "hello"]
    },
    "key3": {
      "variant":"a{sv}",
      "value": {
        "subkey1": {
          "variant":"s",
          "value":"subvalue1"
        },
        "subkey2": {
          "variant":"i",
          "value": 123
        }
      }
    }
  }
}
)*";
    run_one_case(client, request, "method_aDsvD");
  }
  // dbus2http.stop();
  // conn->leaveEventLoop();
  // dbus_thread.join();
}
}  // namespace dbus2http
