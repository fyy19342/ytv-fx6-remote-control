#include "nd_state_machine.h"

#include <algorithm>

namespace {

std::vector<uint64_t> ordered_densities(const std::vector<uint64_t>& available) {
    std::vector<uint64_t> values;
    for (const auto value : available) {
        if (value > 0 && value < 0xffff) values.push_back(value);
    }
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    return values;
}

} // namespace

std::optional<uint64_t> NdStateMachine::brightest(const std::vector<uint64_t>& available) {
    const auto values = ordered_densities(available);
    if (values.empty()) return std::nullopt;
    return values.front();
}

std::optional<uint64_t> NdStateMachine::step(const std::vector<uint64_t>& available,
                                             uint64_t current, int direction) {
    const auto values = ordered_densities(available);
    if (values.empty() || current == 0 || current >= 0xffff) return std::nullopt;
    if (direction == 0) return current;
    if (direction > 0) {
        const auto next = std::upper_bound(values.begin(), values.end(), current);
        return next == values.end() ? values.back() : *next;
    }
    const auto next = std::lower_bound(values.begin(), values.end(), current);
    return next == values.begin() ? values.front() : *std::prev(next);
}
