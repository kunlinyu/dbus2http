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

  bool run(int port) { return server_.listen("0.0.0.0", port); }

  void stop() { server_.stop(); }

private:
  static bool parse_dbus_request_path(const std::string& path,
                                      std::string& service_name,
                                      std::string& object_path,
                                      std::string& interface_name,
                                      std::string& method);

  static std::string replaceAll(const std::string& input, const std::string& from,
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
