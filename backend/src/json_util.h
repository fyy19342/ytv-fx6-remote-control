#pragma once

#include <iomanip>
#include <sstream>
#include <string>

namespace json_util {

inline std::string escape(const std::string& input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        switch (c) {
        case '"': oss << "\\\""; break;
        case '\\': oss << "\\\\"; break;
        case '\b': oss << "\\b"; break;
        case '\f': oss << "\\f"; break;
        case '\n': oss << "\\n"; break;
        case '\r': oss << "\\r"; break;
        case '\t': oss << "\\t"; break;
        default:
            if (c < 0x20) {
                oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c) << std::dec;
            } else {
                oss << c;
            }
        }
    }
    return oss.str();
}

inline std::string quote(const std::string& input) {
    return '"' + escape(input) + '"';
}

inline std::string boolean(bool v) { return v ? "true" : "false"; }

} // namespace json_util
