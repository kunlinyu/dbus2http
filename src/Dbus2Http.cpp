//
// Created by yukunlin on 2025/11/26.
//
#include <dbus2http/Dbus2Http.h>

namespace dbus2http {

Dbus2Http::Dbus2Http(const std::vector<std::string>& service_prefixes,
                     bool system)
    : system_bus_(system) {
  service_prefixes_.insert(service_prefixes.begin(), service_prefixes.end());
  conn_ = DbusUtils::createConnection(system_bus_);
}

void Dbus2Http::start(int port, int ws_port) {
  dbus_caller_ = std::make_unique<DbusCaller>(context_, system_bus_);
  service_ = std::make_unique<WebService>(*dbus_caller_);
  service_thread_ = std::thread([&] { service_->run(port, ws_port); });
  update_thread_ = std::thread([&] {
    while (not stop_) {
      update();
      for (int i = 0; i < 60 and not stop_; ++i)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });
  PLOGI << "dbus2http started on port " << port << "...";
}

void Dbus2Http::stop() {
  stop_ = true;
  PLOGI << "stopping httplib service update thread...";
  if (update_thread_.joinable()) update_thread_.join();
  PLOGI << "httplib service update thread stopped.";
  PLOGI << "stopping httplib service...";
  service_->stop();
  if (service_thread_.joinable()) service_thread_.join();
  PLOGI << "httplib service stopped.";
}

void Dbus2Http::update() {
  DbusEnumerator dbusEnumerator(context_);
  auto service_names = DbusEnumerator::list_services(system_bus_);
  for (const auto& service_name : service_names) {
    if (not match_prefix(service_name)) continue;
    if (context_.contains_service(service_name)) continue;
    PLOGI << "Found new service: " << service_name;
    dbusEnumerator.parse_object_paths_recursively(*conn_.get(), service_name,
                                                  "/");
  }
}

bool Dbus2Http::match_prefix(const std::string& service_name) {
  if (service_prefixes_.empty()) return true;
  for (const auto& prefix : service_prefixes_) {
    if (service_name.rfind(prefix, 0) == 0) return true;
  }
  return false;
}

}  // namespace dbus2http