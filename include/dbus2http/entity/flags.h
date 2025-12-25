//
// Created by yukunlin on 2025/11/27.
//

#pragma once

#include <string>

namespace dbus2http {

enum EmitsChangedSignal {
  TRUE,         // emit signal with new value
  INVALIDATES,  // emit signal without new value
  CONST,        // const property, never changes
  FALSE,        // does not emit signal
};

struct Flags {
  bool deprecated = false;
  bool method_no_reply = false;
  bool privileged = false;
  EmitsChangedSignal emits_changed_signal = TRUE;
  std::string c_symbol;
};

}  // namespace dbus2http
