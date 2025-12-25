//
// Created by yukunlin on 11/22/25.
//
#include <dbus2http/WebService.h>
#include <dbus2http/entity/DbusSerialization.h>

#include <utility>
#include <ranges>

// #embed "../dbus2http_openapi.yaml" as dbus2http_openapi_yaml

namespace dbus2http {

#include "match_rule.inc"

bool WebService::parse_dbus_request_path(const std::string& path,
                                         std::string& service_name,
                                         std::string& object_path,
                                         std::string& interface_name,
                                         std::string& method) {

  std::string rem = path;
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
  std::string header = R"(
<!DOCTYPE html>
<html>
<head>
  <style>
    details, p {
      margin-left: 20px;
      margin-top: 4px;
      margin-bottom: 4px;
    }
  </style>
</head>
<body>
)";
  std::string footer = R"(
</body>
</html>
)";
  std::string try_html = R"(
<!DOCTYPE html>
<html>
  <body>
    <p>service: $service_name$</p>
    <p>object: $object_path$</p>
    <p>interface: $interface_name$</p>
    <p>method: $method_name$</p>
    <p>request signature: $request_signature$</p>
    <p>response signature: $response_signature$</p>
    <div style="display:flex; align-items:center; gap:10px;">
    <textarea id="in" style="width:200px; height:200px;">$request$</textarea>
    <button onclick="
      fetch('$path$',
            { method: 'POST',
              headers: {'Content-Type':'application/json'},
              body: document.getElementById('in').value})
           .then(r=>r.text())
           .then(t=>{
                      try {
                        const jsonObj = JSON.parse(t);
                        document.getElementById('out').value = JSON.stringify(jsonObj, null, 2);
                      } catch (e) {
                        document.getElementById('out').value = t;
                      }
                    })
      ">
      call
    </button>
    <textarea id="out" style="width:200px; height:200px;" readonly></textarea>
    </div>
  </body>
</html>
)";

  bool ret = server_.set_mount_point("/", "/opt/cosmos/var/www/dbus2http");
  if (not ret)
    PLOGW << "set mount point failed";

  // List DBus services
  server_.Get("/dbus/service", [this](const auto& req, auto& res) {
    nlohmann::json j;
    j["services"] = nlohmann::json::array();
    for (const auto& service_name :
         caller_.context().object_paths | std::views::keys) {
      j["services"].push_back(service_name);
    }
    res.set_content(j.dump(), "application/json");
  });
  server_.Get("/dbus/html", [&, header, footer](const auto& req, auto& res) {
    res.set_content(
        header + Dbus2Html::to_html(caller_.context().object_paths) + footer,
        "text/html");
  });
  server_.Get(R"(/dbus/interface/html/(.*))", [&, header, footer](
                                                  const auto& req, auto& res) {
    std::string suffix = req.matches[1];
    Dbus2Html::set_ws_port(std::to_string(ws_port_));
    if (suffix.find('/') == std::string::npos) {
      const std::string& interface_name = suffix;
      if (caller_.context().interfaces.contains(interface_name)) {
        const Interface& interface =
            caller_.context().interfaces.at(interface_name);
        res.set_content(header + Dbus2Html::to_html(interface) + footer,
                        "text/html");
      } else {
        res.status = 404;
        res.set_content("interface not found", "text/plain");
      }
    }
  });
  server_.Get(R"(/dbus/interface/(.*))", [this](const auto& req, auto& res) {
    std::string suffix = req.matches[1];
    if (suffix.find('/') == std::string::npos) {
      const std::string& interface_name = suffix;
      if (caller_.context().interfaces.contains(interface_name)) {
        nlohmann::json j = caller_.context().interfaces.at(interface_name);
        res.set_content(j.dump(2), "application/json");
      } else {
        res.status = 404;
        res.set_content("interface not found", "text/plain");
      }
    }
  });
  server_.Get(R"(/dbus/service/(.*))", [this](const auto& req, auto& res) {
    std::string suffix = req.matches[1];
    if (suffix.find('/') == std::string::npos) {
      const std::string& service_name = suffix;
      if (caller_.context().contains_service(service_name)) {
        nlohmann::json j =
            caller_.context().object_paths_of_service(service_name);
        res.set_content(j.dump(2), "application/json");
      } else {
        res.status = 404;
        res.set_content("service not found", "text/plain");
      }
    }
  });
  server_.Post(R"(/dbus/call/(.*))", [this](const auto& req, auto& res) {
    PLOGD << "in post";
    std::string service_name, object_path, interface_name, method;
    if (!parse_dbus_request_path(req.matches[1], service_name, object_path,
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
  server_.Get(R"(/dbus/try/(.*))", [this, try_html](const auto& req, auto& res) {
    std::string service_name, object_path, interface_name, method;
    if (!parse_dbus_request_path(req.matches[1], service_name, object_path,
                                 interface_name, method)) {
      PLOGE << "parse error";
      res.status = 400;
      res.set_content("invalid or incomplete dbus path", "text/plain");
      return;
    }

    PLOGD << "service_name: " << service_name;
    PLOGD << "object_path: " << object_path;
    PLOGD << "interface_name: " << interface_name;
    PLOGD << "method: " << method;
    try {
      nlohmann::json request = caller_.RandRequest(interface_name, method);
      std::string filled_html =
          replaceAll(try_html, "$path$",
                     "/dbus/call/" + service_name + object_path + "/" +
                         interface_name + "." + method);
      filled_html = replaceAll(filled_html, "$request$", request.dump(2));
      filled_html = replaceAll(filled_html, "$service_name$", service_name);
      filled_html = replaceAll(filled_html, "$object_path$", object_path);
      filled_html = replaceAll(filled_html, "$interface_name$", interface_name);
      filled_html = replaceAll(filled_html, "$method_name$", method);
      const auto& method_type =
          caller_.context().get<Method>(interface_name, method);
      filled_html = replaceAll(filled_html, "$request_signature$", method_type.in_signature());
      filled_html = replaceAll(filled_html, "$response_signature$", method_type.out_signature());
      res.set_content(filled_html, "text/html");
    } catch (const std::exception& e) {
      res.status = 500;
      std::string what = e.what();
      PLOGE << "exception: " << what;
      res.set_content(R"({"message": ")" + what + "\"}", "application/json");
    }
  });
  server_.Get(R"(/dbus/matchrule(.*))", [](const auto&, auto& res) {
    res.status = 200;
    res.set_content(match_rule_html, "text/html");
  });
  server_.set_logger([](const auto& req, const auto& res) {
    // Timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream ts;
    ts << std::put_time(&tm, "%F %T");

    // Log: [timestamp] client "METHOD path" status body_size
    PLOGI << "[" << ts.str() << "] " << req.remote_addr << " \"" << req.method
          << " " << req.path << "\" " << res.status << " "
          << res.body.substr(0, res.body.size() < 200 ? res.body.size() : 200);
    ;
  });

  server_.set_exception_handler([](const httplib::Request& req,
                                   httplib::Response& res,
                                   std::exception_ptr ep) {
    try {
      std::rethrow_exception(std::move(ep));
    } catch (const std::exception& e) {
      PLOGE << "Unhandled exception while serving " << req.path << ": "
            << e.what();
    }
    res.status = 500;
  });
}

}  // namespace dbus2http