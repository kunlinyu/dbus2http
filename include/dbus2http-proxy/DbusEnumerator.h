//
// Created by yukunlin on 2025/11/17.
//

#pragma once

#include "entity/service.h"
#include <string>
#include <vector>

namespace dbus2http {

class DbusEnumerator {
 public:
  DbusEnumerator();

  static std::vector<std::string> list_services() ;

  static std::string introspect_service(const std::string& service_name, const std::string& path) ;
};

}  // namespace dbus2http
