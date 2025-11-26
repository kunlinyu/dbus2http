//
// Created by yukunlin on 2025/11/26.
//
#include <dbus2http/Dbus2Http.h>

namespace dbus2http {

Dbus2Http::Dbus2Http(const std::vector<std::string>& service_prefixes,
                            bool system)
    : system_(system) {
  service_prefixes_.insert(service_prefixes.begin(), service_prefixes.end());
}

void Dbus2Http::start(int port) {
  DbusEnumerator dbusEnumerator(context_);
  auto service_names = DbusEnumerator::list_services();
  for (const auto& service_name : service_names) {
    if (not match_prefix(service_name)) continue;
    PLOGI << "servcie name: " << service_name;
    PLOGI << "object paths: ";
    std::vector<ObjectPath> object_paths =
        dbusEnumerator.parse_object_paths_recursively(service_name, "/");
    object_paths.erase(std::remove_if(object_paths.begin(), object_paths.end(),
                                      [](const ObjectPath& op) {
                                        return op.interfaces.empty();
                                      }),
                       object_paths.end());
    nlohmann::json j;
    for (const auto& op : object_paths) j[op.path] = op;
  }

  dbus_caller_ = std::make_unique<DbusCaller>(context_, system_);
  service_ = std::make_unique<WebService>(*dbus_caller_);
  service_thread_ = std::thread([&] { service_->run(port); });
  PLOGI << "dbus2http started on port " << port << "...";
}

void Dbus2Http::stop() {
  PLOGI << "stopping httplib service...";
  service_->stop();
  if (service_thread_.joinable()) service_thread_.join();
  PLOGI << "httplib service stopped.";
}

bool Dbus2Http::match_prefix(const std::string& service_name) {
  if (service_prefixes_.empty()) return true;
  for (const auto& prefix : service_prefixes_) {
    if (service_name.rfind(prefix, 0) == 0) return true;
  }
  return false;
}

}