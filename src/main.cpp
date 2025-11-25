#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Log.h>

#include <argparse/argparse.hpp>
#include <atomic>
#include <chrono>
#include <csignal>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>

#include "dbus2http/Dbus2Http.h"
#include "dbus2http/EchoService.h"
#include "dbus2http/ExampleService.h"
#include "dbus2http/FileLineFormatter.h"
#include "dbus2http/WebService.h"

static std::atomic_bool g_running{true};

static void handle_sigint(int) { g_running.store(false); }

void RunExample(const std::unique_ptr<sdbus::IConnection>& connection) {
  try {
    dbus2http::ExampleService service(*connection);
    dbus2http::EchoService service2(*connection);

    connection->enterEventLoop();
  } catch (const sdbus::Error& e) {
    PLOGE << "start example service failed: " << e.what();
  }
}

int main(int argc, char* argv[]) {
  // setup signal handlers
  std::signal(SIGINT, handle_sigint);
  std::signal(SIGTERM, handle_sigint);

  // initialize logging
  static plog::ConsoleAppender<dbus2http::FileLineFormatter> consoleAppender;
  plog::init(plog::debug, &consoleAppender);

  // parse arguments
  argparse::ArgumentParser program("dbus2http");
  program.add_description("A D-Bus to HTTP proxy server.");
  program.add_argument("-p", "--port")
      .help("Port to listen on")
      .nargs(1)
      .default_value(8080)
      .scan<'i', int>();
  program.add_argument("--system")
      .help("Use system bus")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("--service_prefix")
      .nargs(argparse::nargs_pattern::at_least_one)
      .help("Only expose services with the given prefix");
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& e) {
    PLOGE << "argument parsing error: " << e.what() << std::endl << program;
    return 1;
  }
  auto service_prefix =
      program.get<std::vector<std::string>>("--service_prefix");
  for (const auto& prefix : service_prefix) PLOGI << "prefix: " << prefix;
  PLOGI << "port: " << program.get<int>("--port");
  PLOGI << "system bus: " << std::to_string(program.get<bool>("--system"));

  // launch example D-Bus service
  const auto conn = dbus2http::DbusUtils::createConnection(
      dbus2http::kExampleServiceName, program.get<bool>("--system"));
  std::thread dbus_thread([&conn] { RunExample(conn); });
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // launch dbus2http proxy
  PLOGI << "Starting dbus2http...";
  dbus2http::Dbus2Http dbus2http(service_prefix, program.get<bool>("--system"));
  dbus2http.start(program.get<int>("--port"));

  while (g_running.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  PLOGI << "Stopping dbus2http...";
  dbus2http.stop();
  PLOGI << "dbus2http stopped.";

  PLOGI << "Stopping D-Bus example service...";
  conn->leaveEventLoop();
  if (dbus_thread.joinable()) dbus_thread.join();
  PLOGI << "D-Bus example service stopped.";

  return 0;
}
