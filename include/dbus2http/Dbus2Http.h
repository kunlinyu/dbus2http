//
// Created by yukunlin on 11/23/25.
//
#pragma once
#include <plog/Log.h>

#include "DbusEnumerator.h"
#include "WebService.h"
#include "entity/InterfaceContext.h"

namespace dbus2http {

class Dbus2Http {
  InterfaceContext context_;
  std::unique_ptr<WebService> service_;
  std::unique_ptr<DbusCaller> dbus_caller_;
  bool system_bus_;
  std::set<std::string> service_prefixes_;

  std::thread service_thread_;
  std::unique_ptr<sdbus::IConnection> conn_;

  std::thread update_thread_;
  bool stop_ = false;

 public:
  Dbus2Http(const std::vector<std::string>& service_prefixes, bool system_bus);

  void start(int port, int ws_port,
             const std::function<void()>& on_failed = nullptr);

  void stop();

  const InterfaceContext& getContext() const { return context_; }

 private:
  bool match_prefix(const std::string& service_name);

  void update();
};

}  // namespace dbus2http