//
// Created by yukunlin on 11/22/25.
//

#include <dbus2http-proxy/SignatureUtils.h>

#include <stdexcept>

namespace dbus2http {

std::vector<std::string> SignatureUtils::split(
    const std::string& sig) {
  std::string remain_sig = sig;
  std::vector<std::string> result;
  std::string::size_type pos;
  while (not remain_sig.empty()) {
    switch (remain_sig.front()) {
      case 'b':  // boolean
      case 'y':  // byte
      case 'n':  // int16
      case 'q':  // uint16
      case 'i':  // int32
      case 'u':  // uint32
      case 'x':  // int64
      case 't':  // uint64
      case 'd':  // double
      case 's':  // string
      case 'v':  // variant
        result.emplace_back(remain_sig.substr(0, 1));
        remain_sig = remain_sig.substr(1);
        break;
      case '(':  // struct
        pos = find_match_bracket(remain_sig, 0, '(', ')');
        result.emplace_back(remain_sig.substr(0, pos));
        remain_sig = remain_sig.substr(pos);
        break;

      case 'a':  // array
        if (remain_sig[1] == '(')
          pos = find_match_bracket(remain_sig, 1, '(', ')');
        else if (remain_sig[1] == '{')
          pos = find_match_bracket(remain_sig, 1, '{', '}');
        else  // single charactor array
          pos = 2;
        result.emplace_back(remain_sig.substr(0, pos));
        remain_sig = remain_sig.substr(pos);
        break;

      case '\0':  // invalid
      case 'h':   // unix file descriptor
      case 'r':   // general concept of struct
      case 'e':   // general concept of dict
      case 'o':   // name of object
      case 'g':   // signature
      case 'm':   // reserved
      case '*':   // reserved
      case '?':   // reserved
      case '@':   // reserved
      case '&':   // reserved
      case '^':   // reserved
        throw std::invalid_argument("Unsupported signature: " +
                                    remain_sig.substr(0, 1));
      default:
        throw std::invalid_argument("Unknown signature: " +
                                    remain_sig.substr(0, 1));
    }
  }
  return result;
}

std::string::size_type SignatureUtils::find_match_bracket(
    const std::string& str, std::string::size_type left_index, char left,
    char right) {
  int count = 1;
  for (std::string::size_type i = left_index + 1; i <= str.length(); i++) {
    if (count == 0) return i;
    if (str[i] == left)
      count++;
    else if (str[i] == right)
      count--;
  }
  return 0;
}

}  // namespace dbus2http