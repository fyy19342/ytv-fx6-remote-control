#pragma once

#include <cstdint>
#include <optional>
#include <vector>

class NdStateMachine {
public:
    // ND density grows as the filter becomes darker. 0xffff means "no value" in the SDK.
    static std::optional<uint64_t> brightest(const std::vector<uint64_t>& available);
    static std::optional<uint64_t> step(const std::vector<uint64_t>& available,
                                        uint64_t current, int direction);
};
