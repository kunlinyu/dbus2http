#include <atomic>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>

#include "dbus2http-proxy/DbusCaller.h"
#include "dbus2http-proxy/DbusEnumerator.h"
#include "dbus2http-proxy/WebService.h"

static std::atomic_bool g_running{true};

static void handle_sigint(int) { g_running.store(false); }

int main() {
  std::signal(SIGINT, handle_sigint);
  std::signal(SIGTERM, handle_sigint);

  // Start web service in background
  dbus2http::WebService web_service;
  // std::thread server_thread([&web_service] {
  //   // blocking call
  //   web_service.run(8080);
  // });

  std::cout << "WebService listening on port 8080. Press Ctrl+C to stop.\n";

  // deadloop controlled by atomic boolean; SIGINT will set g_running to false.
  // while (g_running.load()) {
  //   std::this_thread::sleep_for(std::chrono::milliseconds(200));
  // }

  // std::cout << "Stopping WebService...\n";
  // web_service.stop();
  // if (server_thread.joinable()) server_thread.join();
  // std::cout << "Stopped WebService.\n";

  dbus2http::InterfaceContext context;
  dbus2http::DbusEnumerator dbusEnumerator(context);
  auto service_names = dbus2http::DbusEnumerator::list_services();
  nlohmann::json all;
  for (const auto& service_name : service_names) {
    if (service_name != "com.example.ServiceName") continue;
    std::vector<dbus2http::ObjectPath> object_paths =
        dbusEnumerator.parse_object_paths_recursively(service_name, "/");
    object_paths.erase(std::remove_if(object_paths.begin(), object_paths.end(),
                                      [](const dbus2http::ObjectPath& op) {
                                        return op.interfaces.empty();
                                      }),
                       object_paths.end());
    nlohmann::json j;
    for (const auto& op : object_paths) j[op.path] = op;
        all["services"][service_name] = j;
    all["interfaces"] = context.interfaces;
    std::cout << all.dump(2) << std::endl;
  }

  std::cout << "going to call Method2" << std::endl;
  dbus2http::DbusCaller caller(context);
  nlohmann::json request = {
    {"num", 123},
    {"name2age", {{"Alice", 17}, {"Bob", 18}}},
    {"name", "Charlie"},
    {"valid", true},
    {"name2valid", {{"Alice", true}, {"Bob", false}}}
  };
  std::cout << "request: " << request.dump(2) << std::endl;
  caller.Call("com.example.ServiceName", "/path/to/object",
              "com.example.InterfaceName", "Method2", request);
  std::cout << "call finished" << std::endl;

  // save all into a file
  std::ofstream fout("dbus_structure.json");
  if (fout.is_open()) fout << all.dump(2);
  fout.close();

  return 0;
}
