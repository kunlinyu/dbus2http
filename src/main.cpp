#include <atomic>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>

#include "dbus2http-proxy/DbusCaller.h"
#include "dbus2http-proxy/DbusEnumerator.h"
#include "dbus2http-proxy/ExampleService.h"
#include "dbus2http-proxy/WebService.h"

static std::atomic_bool g_running{true};

static void handle_sigint(int) { g_running.store(false); }

void RunExample(const std::unique_ptr<sdbus::IConnection>& connection) {
  try {
    dbus2http::ExampleService service(*connection);
    std::cout << "service launched" << std::endl;

    connection->enterEventLoop();
    connection->leaveEventLoop();
  } catch (const sdbus::Error& e) {
    std::cerr << e.what() << std::endl;

  }
}

int main() {
  std::signal(SIGINT, handle_sigint);
  std::signal(SIGTERM, handle_sigint);

  const auto conn = sdbus::createSessionBusConnection(
      sdbus::ServiceName(dbus2http::kServiceName));

  std::thread dbus_thread([&conn] { RunExample(conn); });

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
  }

  // Start web service in background
  dbus2http::WebService web_service(context);
  std::thread server_thread([&web_service] { web_service.run(8080); });
  std::cout << "WebService listening on port 8080. Press Ctrl+C to stop.\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  while (g_running.load()) {
    httplib::Client client("http://localhost:8080");
    std::string request = R"(
        {
          "name": "Charlie",
          "name2age": {
            "Alice": 17,
            "Bob": 18
          },
          "name2valid": {
            "Alice": true,
            "Bob": false
          },
          "num": 123,
          "valid": true
        }
      )";
    auto res = client.Post(
        "/dbus/com.example.ServiceName/path/to/object/"
        "com.example.InterfaceName.Method2", request
        , {"Content-Type: application/json"});
    if (res && res->status == 200) {
      std::cout << "dbus Method called." << std::endl;
      std::cout << "Response:\n" << res->body << std::endl;
    } else {
      std::cerr << "dbus Method call failed." << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }

  std::cout << "Stopping WebService...\n";
  web_service.stop();
  if (server_thread.joinable()) server_thread.join();
  std::cout << "Stopped WebService.\n";

  std::cout << "Stopping D-Bus example service...\n";
  conn->leaveEventLoop();
  if (dbus_thread.joinable()) dbus_thread.join();
  std::cout << "Stopped D-Bus example service.\n";

  // save all into a file
  std::ofstream fout("dbus_structure.json");
  if (fout.is_open()) fout << all.dump(2);
  fout.close();

  return 0;
}
