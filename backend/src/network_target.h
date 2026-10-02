#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>

struct NetworkTarget {
    uint32_t sdk_ip = 0;
    // SDK object identifier, not the camera's physical MAC address.
    std::array<uint8_t, 6> object_id{0x02, 0x00, 0, 0, 0, 0};
};

inline std::optional<NetworkTarget> parse_network_target(const std::string& address) {
    NetworkTarget target;
    size_t position = 0;
    for (size_t part = 0; part < 4; ++part) {
        const auto begin = position;
        unsigned value = 0;
        while (position < address.size() && address[position] >= '0' && address[position] <= '9') {
            value = value * 10 + static_cast<unsigned>(address[position++] - '0');
            if (position - begin > 3 || value > 255) return std::nullopt;
        }
        if (position == begin || (position - begin > 1 && address[begin] == '0')) return std::nullopt;
        if (part == 0 && (value == 0 || value == 127 || value >= 224)) return std::nullopt;
        target.sdk_ip |= value << (part * 8);
        target.object_id[part + 2] = static_cast<uint8_t>(value);
        if (part < 3 && (position >= address.size() || address[position++] != '.')) return std::nullopt;
    }
    if (position != address.size()) return std::nullopt;
    return target;
}
