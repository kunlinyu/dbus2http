//
// Created by yukunlin on 11/22/25.
//
#include <dbus2http-proxy/WebService.h>

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

WebService::WebService(const InterfaceContext& context)
    : caller_(context) {
  server_.Post("/echo",
               [](const httplib::Request& req, httplib::Response& res) {
                 res.set_content("echo", "text/plain");
               });

  // Replace the simple "/dbus/" handler with a wildcard handler that parses
  // paths like:
  // /dbus/<service_name>/<object_path...>/<interface_name>/<method>
  server_.Post(R"(/dbus/.*)", [this](const httplib::Request& req,
                                     httplib::Response& res) {
    std::string service_name, object_path, interface_name, method;
    if (!parse_dbus_request_path(req.path, service_name, object_path,
                                 interface_name, method)) {
      std::cerr << "parse error" << std::endl;
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

    // Provide extracted values and leave the rest to the user
    std::cout << "service_name: " << service_name << "\n";
    std::cout << "object_path: " << object_path << "\n";
    std::cout << "interface_name: " << interface_name << "\n";
    std::cout << "method: " << method << "\n";
    nlohmann::json response = caller_.Call(service_name, object_path,
                                           interface_name, method, request);
    res.set_content(response.dump(), "application/json");
  });

  server_.set_logger(
      [](const httplib::Request& req, const httplib::Response& res) {
        // Timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_r(&t, &tm);
        std::ostringstream ts;
        ts << std::put_time(&tm, "%F %T");

        // Log: [timestamp] client "METHOD path" status body_size
        std::cout << "[" << ts.str() << "] " << req.remote_addr << " \""
                  << req.method << " " << req.path << "\" " << res.status << " "
                  << res.body.substr(0, res.body.size() < 100 ? res.body.size() : 100) << std::endl;
      });

  server_.set_exception_handler([](const httplib::Request& req,
                                   httplib::Response& res,
                                   std::exception_ptr ep) {
    try {
      std::rethrow_exception(ep);
    } catch (const std::exception& e) {
      std::cerr << "Unhandled exception while serving " << req.path << ": "
                << e.what() << std::endl;
    }
    res.status = 500;
  });
}

}