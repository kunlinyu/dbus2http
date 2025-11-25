//
// Created by yukunlin on 11/22/25.
//
#include <dbus2http/WebService.h>
#include <dbus2http/entity/DbusSerialization.h>

namespace dbus2http {

bool WebService::parse_dbus_request_path(const std::string& path,
                                         std::string& service_name,
                                         std::string& object_path,
                                         std::string& interface_name,
                                         std::string& method) {
  const std::string prefix = "/dbus/";
  if (path.size() <= prefix.size() ||
      path.compare(0, prefix.size(), prefix) != 0) {
    return false;
  }

  std::string rem =
      path.substr(prefix.size());  // remaining part after "/dbus/"
  const auto first_slash = rem.find('/');
  if (first_slash == std::string::npos) {
    return false;  // missing components
  }
  service_name = rem.substr(0, first_slash);
  std::string rest =
      rem.substr(first_slash + 1);  // everything after service_name/

  const auto last_slash = rest.rfind('/');
  if (last_slash == std::string::npos)
    return false;  // missing interface/method
  std::string interface_method = rest.substr(last_slash + 1);
  std::string before_interface =
      rest.substr(0, last_slash);  // may include object_path and interface

  const auto last_dot = interface_method.rfind('.');
  if (last_dot == std::string::npos) return false;

  method = interface_method.substr(last_dot + 1);
  interface_name = interface_method.substr(0, last_dot);
  object_path = "/";
  if (!before_interface.empty()) object_path += before_interface;
  return true;
}

WebService::WebService(DbusCaller& caller) : caller_(caller) {
  server_.Post("/echo",
               [](const httplib::Request& req, httplib::Response& res) {
                 res.set_content(req.body, "text/plain");
               });
  server_.Get("/hello",
              [](const httplib::Request& req, httplib::Response& res) {
                res.set_content("hello world", "text/plain");
              });
  server_.Get(
      "/dbus", [this](const httplib::Request& req, httplib::Response& res) {
        nlohmann::json j;
        j["services"] = nlohmann::json::array();
        for (const auto& [service_name, _] : caller_.context().object_paths) {
          j["services"].push_back(service_name);
        }
        res.set_content(j.dump(), "application/json");
      });
  server_.Get(R"(/dbus/interface/.*)",
              [this](const httplib::Request& req, httplib::Response& res) {
                std::string suffix = req.path.substr(16);
                if (suffix.find('/') == std::string::npos) {
                  std::string interface_name = suffix;
                  if (caller_.context().interfaces.contains(interface_name)) {
                    nlohmann::json j =
                        caller_.context().interfaces.at(interface_name);
                    res.set_content(j.dump(2), "application/json");
                  } else {
                    res.status = 404;
                    res.set_content("interface not found", "text/plain");
                  }
                }
              });
  server_.Get(R"(/dbus/service/.*)", [this](const httplib::Request& req,
                                    httplib::Response& res) {
    std::string suffix = req.path.substr(14);
    if (suffix.find('/') == std::string::npos) {
      std::string service_name = suffix;
      if (caller_.context().object_paths.contains(service_name)) {
        nlohmann::json j = caller_.context().object_paths.at(service_name);
        res.set_content(j.dump(2), "application/json");
      } else {
        res.status = 404;
        res.set_content("service not found", "text/plain");
      }
    }
  });
  server_.Post(R"(/dbus/.*)", [this](const httplib::Request& req,
                                     httplib::Response& res) {
    PLOGD << "in post";
    std::string service_name, object_path, interface_name, method;
    if (!parse_dbus_request_path(req.path, service_name, object_path,
                                 interface_name, method)) {
      PLOGE << "parse error";
      res.status = 400;
      res.set_content("invalid or incomplete dbus path", "text/plain");
      return;
    }
    nlohmann::json request;
    try {
      request = nlohmann::json::parse(req.body);
    } catch (const nlohmann::json::parse_error& e) {
      res.status = 400;
      res.set_content(std::string("invalid JSON body: ") + e.what(),
                      "text/plain");
      return;
    }

    PLOGD << "service_name: " << service_name;
    PLOGD << "object_path: " << object_path;
    PLOGD << "interface_name: " << interface_name;
    PLOGD << "method: " << method;
    try {
      nlohmann::json response = caller_.Call(service_name, object_path,
                                             interface_name, method, request);
      res.set_content(response.dump(), "application/json");
    } catch (const std::exception& e) {
      res.status = 500;
      std::string what = e.what();
      PLOGE << "exception: " << what;
      res.set_content(R"({"message": ")" + what + "\"}", "application/json");
    }
  });

  server_.set_logger([](const httplib::Request& req,
                        const httplib::Response& res) {
    // Timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_r(&t, &tm);
    std::ostringstream ts;
    ts << std::put_time(&tm, "%F %T");

    // Log: [timestamp] client "METHOD path" status body_size
    PLOGI << "[" << ts.str() << "] " << req.remote_addr << " \"" << req.method
          << " " << req.path << "\" " << res.status << " "
          << res.body.substr(0, res.body.size() < 100 ? res.body.size() : 100);
    ;
  });

  server_.set_exception_handler([](const httplib::Request& req,
                                   httplib::Response& res,
                                   std::exception_ptr ep) {
    try {
      std::rethrow_exception(ep);
    } catch (const std::exception& e) {
      PLOGE << "Unhandled exception while serving " << req.path << ": "
            << e.what();
    }
    res.status = 500;
  });
}

}  // namespace dbus2http