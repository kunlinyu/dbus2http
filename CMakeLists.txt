cmake_minimum_required(VERSION 4.0)
project(dbus2http_proxy)

set(CMAKE_CXX_STANDARD 20)

find_package(sdbus-c++)
find_package(nlohmann_json)
find_package(httplib)

include_directories(include)

add_executable(dbus2http_proxy
        src/main.cpp
        src/DbusEnumerator.cpp)
target_link_libraries(dbus2http_proxy SDBusCpp::sdbus-c++ nlohmann_json::nlohmann_json httplib::httplib)