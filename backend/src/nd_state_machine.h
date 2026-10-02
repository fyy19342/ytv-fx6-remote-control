#pragma once

#include <cstdint>
#include <optional>
#include <vector>

enum class NdValueFormat { OpticalDensity, Transmittance };

class NdStateMachine {
public:
    // ND density grows as the filter becomes darker. 0xffff means "no value" in the SDK.
    static std::optional<uint64_t> brightest(const std::vector<uint64_t>& available,
                                            NdValueFormat format = NdValueFormat::OpticalDensity);
    static std::optional<uint64_t> step(const std::vector<uint64_t>& available,
                                        uint64_t current, int direction,
                                        NdValueFormat format = NdValueFormat::OpticalDensity);
};
