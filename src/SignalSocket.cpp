//
// Created by yukunlin on 2025/11/26.
//

#include "../include/dbus2http/SignalSocket.h"

namespace dbus2http {

using websocketpp::log::alevel;

SignalSocket::SignalSocket(const InterfaceContext& context, bool system, int port)
    : context_(context) {
  ws_server_.init_asio();
  ws_server_.set_open_handler([&](auto conn_hdl) {
    websocketpp::lib::error_code ec;
    server::connection_ptr ws_conn = ws_server_.get_con_from_hdl(conn_hdl, ec);
    if (ec) {
      PLOGE << "get_con_from_hdl error: " << ec.message();
      return;
    }
    const std::string query = ws_conn->get_uri()->get_query();
    if (query.find_first_of("match=") != 0) {
      PLOGE << "invalid query string: " << query;
      ws_server_.close(conn_hdl, websocketpp::close::status::policy_violation,
                       "invalid query string");
      return;
    }

    std::string match = query.substr(6);
    PLOGI << "WebSocket connection opened with query: " << query;
    PLOGD << "match : " << match;
    match = replaceAll(match, "%20", " ");
    match = replaceAll(match, "%27", "'");
    match = replaceAll(match, "%2C", ",");
    match = replaceAll(match, "%3D", "=");
    PLOGD << "match : " << match;
    try {
      conn2slot_[conn_hdl] = dbus_connection_->addMatch(
          match,
          [&, conn_hdl](sdbus::Message msg) {
            PLOGD << "get message from service: " << msg.getInterfaceName()
                  << " member: " << msg.getMemberName();
            std::vector<Argument> args;
            try {
              args =
                  context_
                      .get<Signal>(msg.getInterfaceName(), msg.getMemberName())
                      .args;
            } catch (const std::invalid_argument& e) {
              PLOGW << "unknown signal: " << msg.getInterfaceName() << "."
                    << msg.getMemberName() << ", exception: " << e.what();
              return;
            }

            nlohmann::json j = Message2Json::WrapHeader(
                msg, Message2Json::ExtractMessage(msg, args));
            server::connection_ptr conn =
                ws_server_.get_con_from_hdl(conn_hdl, ec);
            if (conn) {
              ec = conn->send(j.dump(), websocketpp::frame::opcode::text);
              if (ec) {
                PLOGE << "send error: " << ec.message();
              }
            } else
              PLOGD << "connection closed";
          },
          sdbus::return_slot_t());
    } catch (const std::exception& e) {
      PLOGE << "add match failed: " << e.what();
    }
  });
  ws_server_.set_message_handler(
      [&](auto conn_hdl, auto msg) { on_message(&ws_server_, conn_hdl, msg); });
  ws_server_.set_close_handler(
      [&](auto conn_hdl) { conn2slot_.erase(conn_hdl); });
  ws_server_.set_http_handler([](auto conn_hdl) {});
  ws_server_.set_fail_handler([](auto conn_hdl) {});
  ws_server_.set_interrupt_handler([](auto conn_hdl) {});
  ws_server_.set_ping_handler([](auto conn_hdl, auto msg) { return true; });
  ws_server_.set_pong_handler([](auto conn_hdl, auto msg) {});
  ws_server_.set_pong_timeout_handler([](auto conn_hdl, auto msg) {});

  try {
    ws_server_.listen(port);
    ws_server_.start_accept();
  } catch (const std::exception& e) {
    PLOGE << "WebSocket server start failed: " << e.what();
    return;
  }

#ifdef NDEBUG
  ws_server_.clear_access_channels(alevel::all);
  ws_server_.clear_error_channels(alevel::all);
  ws_server_.set_access_channels(alevel::connect | alevel::disconnect |
                                 alevel::fail);
#else
  ws_server_.set_access_channels(alevel::all);
  ws_server_.set_error_channels(alevel::all);
#endif
  dbus_connection_ = DbusUtils::createConnection(system);
}

void SignalSocket::on_message(server* s, websocketpp::connection_hdl conn_hdl,
                              server::message_ptr msg) {
  std::string payload = msg->get_payload();
  s->send(conn_hdl, payload, msg->get_opcode());
}
}  // namespace dbus2http