//
// Created by yukunlin on 2025/11/24.
//
#pragma once
#include <plog/Log.h>

#include <iomanip>

namespace dbus2http {

class FileLineFormatter {
 public:
  static plog::util::nstring header() { return plog::util::nstring(); }
  static plog::util::nstring format(const plog::Record& record) {
    tm t;
    plog::util::localtime_s(&t, &record.getTime().time);
    plog::util::nostringstream ss;
    ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0'))
       << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-")
       << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday
       << PLOG_NSTR(" ");
    ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour
       << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2)
       << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0'))
       << std::setw(2) << t.tm_sec << PLOG_NSTR(".")
       << std::setfill(PLOG_NSTR('0')) << std::setw(3)
       << static_cast<int>(record.getTime().millitm) << PLOG_NSTR(" ");
    ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left
       << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
    ss << PLOG_NSTR("[") << record.getTid() << PLOG_NSTR("] ");
    // ss << PLOG_NSTR("[") << record.getFunc() << PLOG_NSTR("@") <<
    // record.getLine() << PLOG_NSTR("] ");
    ss << PLOG_NSTR("[") << basename(std::string(record.getFile()))
       << PLOG_NSTR(":") << record.getLine() << PLOG_NSTR("] ");
    ss << record.getMessage() << PLOG_NSTR("\n");

    return ss.str();
  }

 private:
  static std::string basename(const std::string& path) {
    size_t last_slash = path.find_last_of('/');
    return (last_slash != std::string::npos) ? path.substr(last_slash + 1)
                                             : path;
  }
};

}  // namespace dbus2http
