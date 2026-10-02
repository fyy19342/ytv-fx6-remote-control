#include "nd_state_machine.h"

#include <algorithm>

namespace {

bool valid(uint64_t value, NdValueFormat format) {
    if (format == NdValueFormat::OpticalDensity) return value > 0 && value < 0xffff;
    const auto numerator = value >> 32;
    const auto denominator = value & 0xffffffff;
    // Reject Clear, unknown and malformed fractions. ND ON needs attenuation.
    return numerator > 0 && numerator < denominator;
}

bool brighter(uint64_t a, uint64_t b, NdValueFormat format) {
    if (format == NdValueFormat::OpticalDensity) return a < b;
    // Cross-products fit in uint64_t for two 32-bit operands, avoiding rounding.
    return (a >> 32) * (b & 0xffffffff) > (b >> 32) * (a & 0xffffffff);
}

std::vector<uint64_t> ordered_densities(const std::vector<uint64_t>& available, NdValueFormat format) {
    std::vector<uint64_t> values;
    for (const auto value : available) {
        if (valid(value, format)) values.push_back(value);
    }
    const auto compare = [format](auto a, auto b) { return brighter(a, b, format); };
    std::stable_sort(values.begin(), values.end(), compare);
    values.erase(std::unique(values.begin(), values.end(), [&](auto a, auto b) {
        return !compare(a, b) && !compare(b, a);
    }), values.end());
    return values;
}

} // namespace

std::optional<uint64_t> NdStateMachine::brightest(const std::vector<uint64_t>& available, NdValueFormat format) {
    const auto values = ordered_densities(available, format);
    if (values.empty()) return std::nullopt;
    return values.front();
}

std::optional<uint64_t> NdStateMachine::step(const std::vector<uint64_t>& available,
                                             uint64_t current, int direction, NdValueFormat format) {
    const auto values = ordered_densities(available, format);
    if (values.empty() || !valid(current, format)) return std::nullopt;
    const auto compare = [format](auto a, auto b) { return brighter(a, b, format); };
    if (direction == 0) return current;
    if (direction > 0) {
        const auto next = std::upper_bound(values.begin(), values.end(), current, compare);
        return next == values.end() ? values.back() : *next;
    }
    const auto next = std::lower_bound(values.begin(), values.end(), current, compare);
    return next == values.begin() ? values.front() : *std::prev(next);
}
