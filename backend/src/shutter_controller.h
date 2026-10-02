#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

// FX6 ShutterSpeedValue is a fraction of a second, not an integer denominator.
inline bool valid_shutter_speed(uint64_t value) {
    const uint64_t numerator = value >> 32;
    const uint64_t denominator = value & 0xffffffff;
    return numerator > 0 && numerator < 0xffffffff && denominator > 0 && denominator < 0xffffffff;
}

inline bool shutter_shorter(uint64_t a, uint64_t b) {
    return (a >> 32) * (b & 0xffffffff) < (b >> 32) * (a & 0xffffffff);
}

inline std::optional<uint64_t> shutter_step(std::vector<uint64_t> values, uint64_t current, int direction) {
    if (!valid_shutter_speed(current)) return std::nullopt;
    values.erase(std::remove_if(values.begin(), values.end(), [](auto v) { return !valid_shutter_speed(v); }), values.end());
    if (values.empty()) return std::nullopt;
    std::stable_sort(values.begin(), values.end(), shutter_shorter);
    values.erase(std::unique(values.begin(), values.end(), [](auto a, auto b) {
        return !shutter_shorter(a, b) && !shutter_shorter(b, a);
    }), values.end());
    // Skip equivalent fractions, and preserve the exact SDK encoding of a step.
    if (direction > 0) {
        for (auto v : values) if (shutter_shorter(current, v)) return v;
    } else if (direction < 0) {
        for (auto it = values.rbegin(); it != values.rend(); ++it) if (shutter_shorter(*it, current)) return *it;
    }
    return current;
}

enum class ShutterProperty { Mode, Speed };
struct ShutterReading {
    bool readable = false;
    bool writable = false;
    uint64_t value = 0;
    std::vector<uint64_t> possible;
};

class ShutterController {
public:
    using Read = std::function<ShutterReading(ShutterProperty)>;
    using Write = std::function<bool(ShutterProperty, uint64_t, std::string&)>;
    ShutterController(Read read, Write write, uint64_t speed_mode)
        : read_(read), write_(write), speed_mode_(speed_mode) {}

    bool step(int direction, std::string& error) {
        error.clear();
        if (direction == 0) return true;
        const auto mode = read_(ShutterProperty::Mode);
        if (!mode.readable) {
            error = "Shutter mode is unavailable. Check the camera connection.";
            return false;
        }
        if (mode.value != speed_mode_ && !apply(ShutterProperty::Mode, speed_mode_, error)) return false;
        const auto speed = read_(ShutterProperty::Speed);
        const auto target = shutter_step(speed.possible, speed.value, direction);
        if (!speed.readable || !speed.writable || !target) {
            error = "No writable shutter speed steps. Set the camera to manual shutter Speed mode. Mode may have changed.";
            return false;
        }
        if (!apply(ShutterProperty::Speed, *target, error)) return false;
        const auto after = read_(ShutterProperty::Mode);
        if (!after.readable || after.value != speed_mode_) {
            error = "Shutter mode changed during adjustment. Check the camera state.";
            return false;
        }
        return true;
    }

private:
    bool apply(ShutterProperty property, uint64_t value, std::string& error) {
        const auto before = read_(property);
        if (before.readable && before.value == value) return true;
        if (!before.writable || std::find(before.possible.begin(), before.possible.end(), value) == before.possible.end()) {
            error = "Requested shutter setting is unavailable. Set the camera to manual shutter Speed mode.";
            return false;
        }
        if (!write_(property, value, error)) return false;
        const auto after = read_(property);
        if (!after.readable || after.value != value) {
            error = "Shutter readback did not confirm the requested setting. Check the camera state.";
            return false;
        }
        return true;
    }
    Read read_;
    Write write_;
    uint64_t speed_mode_;
};
