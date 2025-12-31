//
// Created by yukunlin on 2025/12/31.
//

#pragma once

#include <cstddef>

namespace dbus2http {

struct Config {
  bool system_bus = true;
  bool binary_support = false;
  size_t max_file_descriptor_size = 10 * 1024 * 1024;  // 10MB
};

}  // namespace dbus2http
