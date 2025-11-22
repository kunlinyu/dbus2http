cmake_minimum_required(VERSION 3.22)
project(dbus2http_proxy)

set(CMAKE_CXX_STANDARD 20)

find_package(sdbus-c++ REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(httplib REQUIRED)
find_package(tinyxml2 REQUIRED)

include_directories(include)

add_executable(dbus2http_proxy
        src/main.cpp
        src/DbusEnumerator.cpp)
target_link_libraries(dbus2http_proxy tinyxml2::tinyxml2 SDBusCpp::sdbus-c++ nlohmann_json::nlohmann_json httplib::httplib)
