#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

enum class ExposureKind { Iris, Iso };

inline uint64_t exposure_number(ExposureKind kind, uint64_t raw) {
    return kind == ExposureKind::Iso ? raw & 0x00ffffff : raw;
}

inline bool valid_exposure(ExposureKind kind, uint64_t raw) {
    const auto value = exposure_number(kind, raw);
    return kind == ExposureKind::Iris ? value > 0 && value <= 0xfffd
                                     : raw <= 0xffffffff && value > 0 && value != 0x00ffffff;
}

// Return the immediate numeric neighbour, keeping the SDK's ISO mode bits.
inline std::optional<uint64_t> exposure_step(ExposureKind kind,
        std::vector<uint64_t> values, uint64_t current, int direction) {
    values.erase(std::remove_if(values.begin(), values.end(), [kind](auto v) {
        return !valid_exposure(kind, v);
    }), values.end());
    if (values.empty() || !valid_exposure(kind, current)) return std::nullopt;
    std::stable_sort(values.begin(), values.end(), [kind](auto a, auto b) {
        return exposure_number(kind, a) < exposure_number(kind, b);
    });
    const auto number = exposure_number(kind, current);
    if (direction > 0) {
        for (auto value : values) if (exposure_number(kind, value) > number) return value;
        return current;
    }
    if (direction < 0) {
        for (auto it = values.rbegin(); it != values.rend(); ++it)
            if (exposure_number(kind, *it) < number) return *it;
    }
    return current;
}
