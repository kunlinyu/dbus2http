#!/bin/bash

conan install . --profile:host=docker/conan/profile_host_x86_64.txt
cmake --preset x86_64-Release
cmake --build --preset x86_64-Release

conan install . --profile:host=docker/conan/profile_host_x86_64.txt --settings:host="build_type=Debug"
cmake --preset x86_64-Debug
cmake --build --preset x86_64-Debug

conan install . --profile:host=docker/conan/profile_host_armv8.txt
cmake --preset armv8-Release
cmake --build --preset armv8-Release

cpack -G DEB --config build/armv8/Release/dbus2httpCPackConfig.cmake