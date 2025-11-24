//
// Created by yukunlin on 2025/11/18.
//

#pragma once

#include <httplib/httplib.h>

#include <string>
#include "DbusCaller.h"

namespace dbus2http {

class WebService {
  httplib::Server server_;
  DbusCaller& caller_;

 public:
  explicit WebService(DbusCaller& caller);

  bool run(int port) {
    return server_.listen("0.0.0.0", port);
  }

  void stop() { server_.stop(); }

  static bool parse_dbus_request_path(const std::string& path,
                                    std::string& service_name,
                                    std::string& object_path,
                                    std::string& interface_name,
                                    std::string& method);
};

}  // namespace dbus2http
