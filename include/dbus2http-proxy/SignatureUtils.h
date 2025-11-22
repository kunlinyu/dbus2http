//
// Created by yukunlin on 11/22/25.
//

#pragma once

#include <string>
#include <vector>

namespace dbus2http {

class SignatureUtils {
 public:
  static std::vector<std::string> split(const std::string& sig);

  static std::string::size_type find_match_bracket(
      const std::string& str, std::string::size_type left_index, char left,
      char right);
};

}  // namespace dbus2http
