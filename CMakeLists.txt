cmake_minimum_required(VERSION 3.22)
project(dbus2http_proxy)

set(CMAKE_CXX_STANDARD 20)

find_package(sdbus-c++ REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(httplib REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(Catch2 REQUIRED)

include_directories(include)

add_library(libdbus2http
        src/DbusEnumerator.cpp
        src/DbusSerialization.cpp
        src/SignatureUtils.cpp
        src/WebService.cpp
        src/Json2Message.cpp
        src/ExampleService.cpp)
target_link_libraries(libdbus2http tinyxml2::tinyxml2 SDBusCpp::sdbus-c++ nlohmann_json::nlohmann_json httplib::httplib)

add_executable(dbus2http src/main.cpp)
target_link_libraries(dbus2http libdbus2http)

add_executable(test-dbus2http tests/test_methods.cpp)
target_link_libraries(test-dbus2http libdbus2http tinyxml2::tinyxml2 SDBusCpp::sdbus-c++ nlohmann_json::nlohmann_json httplib::httplib Catch2::Catch2WithMain)
