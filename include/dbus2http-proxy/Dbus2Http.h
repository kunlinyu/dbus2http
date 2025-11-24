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
  std::unique_ptr<DbusCaller> dbus_caller_;
  bool system_;
  std::set<std::string> service_prefixes_;

  std::thread service_thread_;
public:
  Dbus2Http(const std::vector<std::string>& service_prefixes, bool system) : system_(system) {
    service_prefixes_.insert(service_prefixes.begin(), service_prefixes.end());
  }

  void start(int port) {
    DbusEnumerator dbusEnumerator(context_);
    auto service_names = dbus2http::DbusEnumerator::list_services();
    for (const auto& service_name : service_names) {
      if (not match_prefix(service_name)) continue;
      std::cout << "servcie name: " << service_name << std::endl;
      std::cout << "object paths: " << std::endl;
      std::vector<ObjectPath> object_paths =
          dbusEnumerator.parse_object_paths_recursively(service_name, "/");
      object_paths.erase(std::remove_if(object_paths.begin(), object_paths.end(),
                                        [](const dbus2http::ObjectPath& op) {
                                          return op.interfaces.empty();
                                        }),
                         object_paths.end());
      nlohmann::json j;
      for (const auto& op : object_paths) j[op.path] = op;
    }

    dbus_caller_ = std::make_unique<DbusCaller>(context_, system_);
    service_ = std::make_unique<WebService>(*dbus_caller_);
    service_thread_ = std::thread([&] { service_->run(port); });
    std::cout << "dbus2http started on port " << port << "..." << std::endl;
  }

  void stop() {
    std::cout << "stopping httplib service..." << std::endl;
    service_->stop();
    if (service_thread_.joinable()) service_thread_.join();
    std::cout << "httplib service stopped." << std::endl;
  }

private:
  bool match_prefix(const std::string& service_name) {
    if (service_prefixes_.empty()) return true;
    for (const auto& prefix : service_prefixes_) {
      if (service_name.rfind(prefix, 0) == 0) return true;
    }
    return false;
  }

};

}