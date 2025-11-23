//
// Created by yukunlin on 11/23/25.
//
#pragma once
#include "DbusEnumerator.h"
#include "WebService.h"
#include "entity/InterfaceContext.h"

namespace dbus2http {

class Dbus2Http {
  InterfaceContext context_;
  std::unique_ptr<WebService> service_;

  std::thread service_thread_;
public:
  Dbus2Http() {
  }

  void start() {
    DbusEnumerator dbusEnumerator(context_);
    auto service_names = dbus2http::DbusEnumerator::list_services();
    for (const auto& service_name : service_names) {
      if (service_name != "com.example.ServiceName" and service_name != "com.test.ServiceName") continue;
      std::cout << service_name << std::endl;
      std::vector<dbus2http::ObjectPath> object_paths =
          dbusEnumerator.parse_object_paths_recursively(service_name, "/");
      object_paths.erase(std::remove_if(object_paths.begin(), object_paths.end(),
                                        [](const dbus2http::ObjectPath& op) {
                                          return op.interfaces.empty();
                                        }),
                         object_paths.end());
      nlohmann::json j;
      for (const auto& op : object_paths) j[op.path] = op;
    }

    service_ = std::make_unique<WebService>(context_);
    service_thread_ = std::thread([&] { service_->run(8080); });
  }

  void stop() {
    std::cout << "Stopping WebService...\n";
    service_->stop();
    if (service_thread_.joinable()) service_thread_.join();
    std::cout << "Stopped WebService.\n";
  }


};

}