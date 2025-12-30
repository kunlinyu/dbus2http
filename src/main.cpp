#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
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
#include "dbus2http/SignalSocket.h"
#include "dbus2http/WebService.h"

#ifndef VERSION_BUILD_NUMBER
#define VERSION_BUILD_NUMBER "0.0.0.000"
#endif

#ifndef REVISION
#define REVISION "unknown"
#endif

#ifndef TOOLCHAIN
#define TOOLCHAIN "c++"
#endif

static std::atomic_bool g_running{true};

static void handle_sigint(int) { g_running.store(false); }

void RunExample(const std::unique_ptr<sdbus::IConnection>& connection) {
  try {
    dbus2http::ExampleService service(*connection);
    dbus2http::EchoService service2(*connection);

    service.start();
    connection->enterEventLoop();
    service.stop();
  } catch (const sdbus::Error& e) {
    PLOGE << "start example service failed: " << e.what();
  }
}

int main(int argc, char* argv[]) {
  // setup signal handlers
  std::signal(SIGINT, handle_sigint);
  std::signal(SIGTERM, handle_sigint);

  // parse arguments
  argparse::ArgumentParser program("dbus2http", VERSION_BUILD_NUMBER);
  program.add_description("A D-Bus to HTTP proxy server.");
  program.add_argument("-p", "--port")
      .help("Port to listen on")
      .nargs(1)
      .default_value(10059)
      .scan<'i', int>();
  program.add_argument("-wsp", "--websocket_port")
      .help("Websocket port to listen on")
      .nargs(1)
      .default_value(10058)
      .scan<'i', int>();
  program.add_argument("--system")
      .help("Use system bus")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("--service_prefix")
      .nargs(argparse::nargs_pattern::at_least_one)
      .help("Only expose services with the given prefix");
  program.add_argument("-v", "--verbose")
      .help("print debug log")
      .default_value(false)
      .implicit_value(true);

  static plog::ColorConsoleAppender<dbus2http::FileLineFormatter<false, false>>
      consoleAppender;
  static plog::RollingFileAppender<dbus2http::FileLineFormatter<true, true>> fileAppender("/var/log/dbus2http/dbus2http.log", 10000000, 5);

  try {
    program.parse_args(argc, argv);
#ifdef NDEBUG
    if (program.get<bool>("--verbose"))
      plog::init(plog::debug, &consoleAppender).addAppender(&fileAppender);
    else
      plog::init(plog::info, &consoleAppender).addAppender(&fileAppender);
#else
    plog::init(plog::debug, &consoleAppender);
#endif
  } catch (const std::exception& e) {
    PLOGE << "argument parsing error: " << e.what() << std::endl << program;
    return 1;
  }

  PLOGI << "================================";
  PLOGI << "dbus2http version: " << VERSION_BUILD_NUMBER;
  PLOGI << "revision: " << REVISION;
  PLOGI << "build time: " << std::string(__DATE__ " " __TIME__);
  PLOGI << "toolchain: " << std::string(TOOLCHAIN);

#ifdef NDEBUG
  PLOGI << "build mode: release";
#else
  PLOGI << "build mode: debug";
#endif
  auto service_prefix =
      program.get<std::vector<std::string>>("--service_prefix");
  for (const auto& prefix : service_prefix) PLOGI << "prefix: " << prefix;
  PLOGI << "http port: " << program.get<int>("--port");
  PLOGI << "websocket port: " << program.get<int>("--websocket_port");
  PLOGI << "system bus: " << (program.get<bool>("--system") ? "true" : "false");
  PLOGI << "================================";

  // launch example D-Bus service
  // const auto conn = dbus2http::DbusUtils::createConnection(
  //     dbus2http::kExampleServiceName, program.get<bool>("--system"));
  // std::thread dbus_thread([&conn] { RunExample(conn); });
  // std::this_thread::sleep_for(std::chrono::milliseconds(10));

  std::this_thread::sleep_for(std::chrono::seconds(5));

  // launch dbus2http proxy
  PLOGI << "Starting dbus2http...";
  dbus2http::Dbus2Http dbus2http(service_prefix, program.get<bool>("--system"));
  dbus2http.start(program.get<int>("--port"),
                  program.get<int>("--websocket_port"),
                  [] { g_running.store(false); });

  dbus2http::SignalSocket signal_socket(dbus2http.getContext(),
                                        program.get<bool>("--system"),
                                        program.get<int>("--websocket_port"));
  signal_socket.start();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  PLOGI << "dbus2http started on port " << program.get<int>("--port") << "...";

  int i = 0;
  while (g_running.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (i++ % 10 == 0)
      PLOGD << "dbus2http keep running...";
  }

  PLOGI << "Stopping signal socket...";
  signal_socket.stop();
  PLOGI << "signal socket stopped.";

  PLOGI << "Stopping dbus2http...";
  dbus2http.stop();
  PLOGI << "dbus2http stopped.";

  // PLOGI << "Stopping D-Bus example service...";
  // conn->leaveEventLoop();
  // if (dbus_thread.joinable()) dbus_thread.join();
  // PLOGI << "D-Bus example service stopped.";

  return 0;
}
