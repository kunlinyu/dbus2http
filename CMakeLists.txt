cmake_minimum_required(VERSION 3.15)
project(dbus2http
        VERSION 0.1.0
        DESCRIPTION "A proxy to exposes existing DBus services over HTTP/JSON"
        HOMEPAGE_URL "https://github.com/kunlinyu/dbus2http"
        LANGUAGES CXX
)

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

set(VERSION_FULL ${PROJECT_VERSION}.${BUILD_NUMBER})
add_definitions(-DVERSION_BUILD_NUMBER="${VERSION_FULL}")
include(${CMAKE_SOURCE_DIR}/cmake/git_head.cmake)
get_git_head(REVISION)
message(STATUS "REVISION: ${REVISION}")
add_definitions(-DREVISION="${REVISION}")
message(STATUS "TOOLCHAIN: ${TOOLCHAIN}")
add_definitions(-DTOOLCHAIN="${TOOLCHAIN}")
