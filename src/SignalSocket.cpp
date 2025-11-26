//
// Created by yukunlin on 2025/11/26.
//

#include "../include/dbus2http/SignalSocket.h"

namespace dbus2http {

SignalSocket::SignalSocket(const InterfaceContext& context, bool system)
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
    PLOGI << "match : " << match;
    match = replaceAll(match, "%27", "'");
    PLOGI << "match : " << match;
    sdbus::return_slot_t resurn_slot;
    conn2slot_[conn_hdl] = dbus_connection_->addMatch(match, [&, conn_hdl](sdbus::Message msg) {
      auto args = context_.interfaces.at(msg.getInterfaceName())
                      .get_signal(msg.getMemberName())
                      .args;
      PLOGD << "get message from service: " << msg.getInterfaceName() << " member: " << msg.getMemberName();
      nlohmann::json j = Message2Json::WrapHeader(
          msg, Message2Json::ExtractMessage(msg, args));
      server::connection_ptr conn = ws_server_.get_con_from_hdl(conn_hdl, ec);
      if (conn)
        conn->send(j.dump(), websocketpp::frame::opcode::text);
      else
        PLOGD << "connection closed";
    }, resurn_slot);
  });
  ws_server_.set_message_handler(
      [&](auto conn_hdl, auto msg) { on_message(&ws_server_, conn_hdl, msg); });

  ws_server_.set_close_handler([&](auto conn_hdl) {
    conn2slot_.erase(conn_hdl);
  });
  ws_server_.set_http_handler([](auto conn_hdl) {});
  ws_server_.set_fail_handler([](auto conn_hdl) {});
  ws_server_.set_interrupt_handler([](auto conn_hdl) {});
  ws_server_.set_ping_handler([](auto conn_hdl, auto msg) { return true; });
  ws_server_.set_pong_handler([](auto conn_hdl, auto msg) {});
  ws_server_.set_pong_timeout_handler([](auto conn_hdl, auto msg) {});

  ws_server_.listen(9002);

  ws_server_.start_accept();

  dbus_connection_ = DbusUtils::createConnection(system);
}

void SignalSocket::on_message(server* s, websocketpp::connection_hdl conn_hdl,
                              server::message_ptr msg) {
  std::string payload = msg->get_payload();
  s->send(conn_hdl, payload, msg->get_opcode());
}
}  // namespace dbus2http