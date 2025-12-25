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

## HTTP API

This project exposes a small HTTP API that maps HTTP requests to DBus operations. Main endpoints (as defined in dbus2http_openapi.yaml):

1) GET /dbus/service
- Purpose: list DBus services visible on the system bus.
- Response: JSON array of service name strings.

2) GET /dbus/service/{service}
- Purpose: list object paths for a given service.
- Path parameter:
  - service (string): DBus service name (e.g. org.freedesktop.NetworkManager)
- Response: JSON object whose keys are object path strings and values are ObjectPath objects.
  - ObjectPath:
    - interfaces: array of strings — interfaces implemented at that path
  - Example:
    {
      "/org/freedesktop/NetworkManager": { "interfaces": ["org.freedesktop.NetworkManager", "org.freedesktop.DBus.Introspectable"] },
      "/org/freedesktop/NetworkManager/Devices/0": { "interfaces": ["org.freedesktop.NetworkManager.Device"] }
    }

3) GET /dbus/interface/{interface}
- Purpose: return the introspected representation of a single interface as JSON.
- Path parameter:
  - interface (string): interface name (e.g. org.freedesktop.NetworkManager)
- Response: Interface object (see schema below).

Interface schema (JSON shape returned by GET /dbus/interface/{interface})
- name: string
- methods: object — mapping from method name to Method object
- signals: object — mapping from signal name to Signal object
- properties: object — mapping from property name to Property object
- flags: Flags (optional metadata)

Method
- name: string
- args: array of Argument — combined in/out argument list (each Argument has direction)
- flags: Flags
- Example method:
  {
    "name": "GetDevices",
    "args": [],
    "flags": { "method_no_reply": false, "deprecated": false }
  }

Signal
- name: string
- args: array of Argument
- flags: Flags

Property
- name: string
- type: string — DBus type signature (e.g. "s", "i", "a{sv}")
- access: string — "read", "write", or "readwrite"
- flags: Flags

Argument
- name: string
- type: string — DBus type signature
- direction: string — "in" or "out"

Flags
- deprecated: boolean
- method_no_reply: boolean
- privileged: boolean
- emits_changed_signal: one of "TRUE" | "INVALIDATES" | "CONST" | "FALSE"
- c_symbol: string (optional)

4) POST /dbus/call/{service}/{object_path}/{interface}/{method}
- Purpose: generic DBus method invocation endpoint (documented in OpenAPI).
- Path parameters:
  - service (string): DBus service name
  - object_path (string): object path (leading '/'), may contain additional '/' characters
  - interface (string): interface name
  - method (string): method name
- Request body: MethodCall (free-form JSON mapping argument name -> value)
- Response: MethodReply (free-form JSON; if single basic value, may appear as "result")

MethodCall / MethodReply
- MethodCall: free-form object; server adapts JSON to DBus types using introspection signatures.
- MethodReply: free-form object representing the method return value(s).

Notes on path parsing
- object_path may contain '/' characters. When constructing requests clients can URL-encode path segments.
- The OpenAPI path for method invocation uses separate path parameters to avoid ambiguous captures; clients should follow the documented encoding rules.
