//
// Created by yukunlin on 2025/11/27.
//

#pragma once
#include <plog/Log.h>
#include <sdbus-c++/Message.h>

#include <chrono>
#include <nlohmann/json.hpp>
#include <random>

#include "SignatureUtils.h"
#include "entity/argument.h"

namespace dbus2http {
class Rand2Json {
  static inline std::mt19937 gen = std::mt19937(static_cast<unsigned int>(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  static inline std::uniform_int_distribution<int> dist_pos_ =
      std::uniform_int_distribution<int>(0, 100);
  static inline std::uniform_int_distribution<int> dist_neg_ =
      std::uniform_int_distribution<int>(-100, 5);
  static inline std::uniform_int_distribution<size_t> dist_repeat_ =
      std::uniform_int_distribution<size_t>(2, 3);

 public:
  static nlohmann::json RandJson(const std::vector<Argument>& args);

  static nlohmann::json RandVariant();

  static nlohmann::json RandJson(const std::string& sig);
};
}  // namespace dbus2http