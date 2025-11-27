#pragma once

#include <string>

#include "flags.h"
#include "method.h"
#include "property.h"
#include "signal.h"

namespace dbus2http {

struct Interface {
  std::string name;
  std::map<std::string, Method> methods;
  std::map<std::string, Signal> signals;
  std::map<std::string, Property> properties;
  Flags flags;

  Interface() = default;

  template <typename T>
  void add(const T& m) {
    get_container<T>()[m.name] = m;
  }

  template <typename T>
  [[nodiscard]] const T& get(const std::string& member_name) const {
    if (get_container<T>().contains(member_name))
      return get_container<T>().at(member_name);
    throw std::invalid_argument("member not found: " + member_name);
  }

 private:
  template <typename T>
  std::map<std::string, T>& get_container() {
    return const_cast<std::map<std::string, T>&>(
        static_cast<const Interface*>(this)->get_container<T>());
  }
  template <typename T>
  const std::map<std::string, T>& get_container() const {
    if constexpr (std::is_same_v<T, Method>) {
      return methods;
    } else if constexpr (std::is_same_v<T, Signal>) {
      return signals;
    } else if constexpr (std::is_same_v<T, Property>) {
      return properties;
    } else {
      throw std::invalid_argument("unknown member type");
    }
  }
};

}  // namespace dbus2http
