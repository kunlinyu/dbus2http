cmake_minimum_required(VERSION 3.15)
project(dbus2http_proxy)

set(CMAKE_CXX_STANDARD 20)

find_package(sdbus-c++ REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(httplib REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(argparse REQUIRED)
find_package(websocketpp REQUIRED)

find_package(plog REQUIRED)
add_compile_definitions(PLOG_CAPTURE_FILE)

find_package(Catch2)

include_directories(include)

add_library(libdbus2http
        src/DbusEnumerator.cpp
        src/DbusSerialization.cpp
        src/SignatureUtils.cpp
        src/WebService.cpp
        src/Json2Message.cpp
        src/Message2Json.cpp
        src/ExampleService.cpp
        src/Dbus2Http.cpp
        src/SignalSocket.cpp
        src/Rand2Json.cpp
)
target_link_libraries(libdbus2http plog::plog tinyxml2::tinyxml2 SDBusCpp::sdbus-c++ nlohmann_json::nlohmann_json httplib::httplib websocketpp::websocketpp)

add_executable(dbus2http src/main.cpp)
target_link_libraries(dbus2http libdbus2http argparse::argparse)

add_executable(test-dbus2http tests/test_methods.cpp)
target_link_libraries(test-dbus2http libdbus2http Catch2::Catch2WithMain)
