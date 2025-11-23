# dbus2http
A C++ open-source project that exposes existing DBus services over HTTP/JSON, enabling cross-language and cross-network access to DBus methods without direct DBus integration. Uses sdbus-cpp for DBus introspection/interaction and httplib.h (a header-only C++ HTTP library) for HTTP server functionality.

# Core Concept
dbus2http dynamically discovers DBus services, object paths, and methods via DBus introspection. It acts as a bridge:
Converts incoming HTTP/JSON requests to DBus method calls (using DBus method signatures for type mapping)
Executes the DBus method on the target service/object
Converts DBus method responses back to JSON and returns them via HTTP

# Features
* Dynamic DBus Introspection: Recursively enumerates DBus services, object paths, and methods from the session/system bus
* HTTP Method Invocation: Expose DBus methods via HTTP POST requests

# Dependencies
C++17 or later (required for sdbus-cpp and httplib.h)
sdbus-cpp: DBus library for C++ (with introspection support)
httplib.h: Header-only C++ HTTP server/client library (included as a submodule)
nlohmann/json: JSON serialization/deserialization (header-only)
CMake 3.16+ (build system)
