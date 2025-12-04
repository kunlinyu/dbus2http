# dbus2http
A C++ open-source project that exposes existing DBus services over HTTP/JSON, enabling cross-language and cross-network access to DBus methods without direct DBus integration. Uses sdbus-cpp for DBus introspection/interaction and httplib.h (a header-only C++ HTTP library) for HTTP server functionality.

# Core Concept
dbus2http dynamically discovers DBus services, object paths, and methods via DBus introspection. It acts as a bridge:
* Converts incoming HTTP/JSON requests to DBus method calls (using DBus method signatures for type mapping)
* Executes the DBus method on the target service/object
* Converts DBus method responses back to JSON and returns them via HTTP

# Features
* Dynamic DBus Introspection: Recursively enumerates all available DBus services, object paths, interfaces, and their methods, signals, and properties from the session or system bus using the Introspect method.
* HTTP Discovery Endpoint: Exposes an HTTP interface (GET /introspect) to retrieve the complete, scanned introspection data in a structured JSON format, providing a live overview of the available DBus ecosystem.
* HTTP Method Invocation: Call any DBus method via an HTTP POST request. dbus2http automatically converts your JSON payload into the correct DBus message types, executes the call, and converts the response back to JSON.
* DBus Signal Monitoring: Subscribe to DBus signals using a WebSocket connection. Provide a DBus match rule to filter signals, and dbus2http will forward matching signals as JSON messages over the WebSocket in real-time.

# Dependencies
* C++17 or later (required for sdbus-cpp and httplib.h)
* sdbus-cpp: DBus library for C++ (with introspection support)
* cpp-httplib: Header-only C++ HTTP server/client library (included as a submodule)
* nlohmann/json: JSON serialization/deserialization (header-only)
* CMake 3.19+ (build system)
