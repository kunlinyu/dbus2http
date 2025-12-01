//
// Created by yukunlin on 2025/11/26.
//

#pragma once

#include <plog/Log.h>
#include <sdbus-c++/IConnection.h>

#include <nlohmann/json_fwd.hpp>
#include <thread>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "DbusUtils.h"
#include "Message2Json.h"
#include "entity/InterfaceContext.h"

namespace dbus2http {

typedef websocketpp::server<websocketpp::config::asio> server;
struct ConnectionHdlCompare {
  bool operator()(const websocketpp::connection_hdl& a,
                  const websocketpp::connection_hdl& b) const {
    auto ptr_a = a.lock();
    auto ptr_b = b.lock();
    if (!ptr_a || !ptr_b) return !ptr_a && ptr_b;
    return ptr_a < ptr_b;
  }
};

struct ConnectionContext {
  websocketpp::connection_hdl conn_hdl;
  std::string match;
  server::connection_ptr conn;
};

class SignalSocket {
  server ws_server_;
  std::thread ws_server_thread_;
  std::thread dbus_session_thread_;
  std::map<websocketpp::connection_hdl, sdbus::Slot, ConnectionHdlCompare>
      conn2slot_;

  std::unique_ptr<sdbus::IConnection> dbus_connection_;
  const InterfaceContext& context_;

 public:
  SignalSocket(const InterfaceContext& context, bool system, int port);

  void start() {
    ws_server_thread_ = std::thread([&]() { ws_server_.run(); });
    dbus_session_thread_ =
        std::thread([&]() { dbus_connection_->enterEventLoop(); });
  }

  void stop() {
    ws_server_.stop_listening();
    ws_server_.stop();
    if (ws_server_thread_.joinable()) ws_server_thread_.join();
    dbus_connection_->leaveEventLoop();
    if (dbus_session_thread_.joinable()) dbus_session_thread_.join();
  }

  void on_message(server* s, websocketpp::connection_hdl conn_hdl,
                  server::message_ptr msg);

 private:
  std::string replaceAll(const std::string& input, const std::string& from,
                         const std::string& to) {
    std::string result = input;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
      result.replace(pos, from.length(), to);
      pos += to.length();
    }
    return result;
  }
};

}  // namespace dbus2http
