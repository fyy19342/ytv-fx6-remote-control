#include <cstdlib>
#include <iostream>

#include "nd_state_machine.h"

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << std::endl;
        std::exit(1);
    }
}

} // namespace

int main() {
    const std::vector<uint64_t> values = {210, 120, 0xffff, 60, 90, 0, 120};
    require(NdStateMachine::brightest(values) == 60, "ND ON must choose the brightest valid step");
    require(NdStateMachine::step(values, 120, -1) == 90, "u must brighten ND by one step");
    require(NdStateMachine::step(values, 120, 1) == 210, "d must darken ND by one step");
    require(NdStateMachine::step(values, 60, -1) == 60, "u must stop at the brightest step");
    require(NdStateMachine::step(values, 210, 1) == 210, "d must stop at the darkest step");
    require(!NdStateMachine::brightest({0, 0xffff}).has_value(), "invalid values must be ignored");
    require(!NdStateMachine::brightest({}).has_value(), "empty list is unavailable");
    require(NdStateMachine::step(values, 100, -1) == 90, "in-between value must not skip a brighter step");
    require(NdStateMachine::step(values, 100, 1) == 120, "in-between value must not skip a darker step");
    require(!NdStateMachine::step(values, 0xffff, 1), "unknown current value must not be stepped");
    require(NdStateMachine::step({90}, 90, 1) == 90, "single density clamps");

    const auto fraction = [](uint64_t numerator, uint64_t denominator) { return (numerator << 32) | denominator; };
    const auto format = NdValueFormat::Transmittance;
    const std::vector<uint64_t> transmission = {fraction(1, 128), fraction(1, 8), fraction(1, 4),
        fraction(10, 48), fraction(2, 8), 0, UINT64_MAX, fraction(1, 1), fraction(1, 0)};
    require(NdStateMachine::brightest(transmission, format) == fraction(1, 4), "FX6 ON chooses 1/4 by ratio");
    require(NdStateMachine::step(transmission, fraction(1, 4), 1, format) == fraction(10, 48),
            "FX6 darkens to 1/4.8, retaining the exact SDK numerator/denominator");
    require(NdStateMachine::step(transmission, fraction(10, 48), -1, format) == fraction(1, 4),
            "FX6 brightens one step and deduplicates equal fractions");
    require(NdStateMachine::step(transmission, fraction(1, 4), -1, format) == fraction(1, 4), "FX6 minimum clamps");
    require(NdStateMachine::step(transmission, fraction(1, 128), 1, format) == fraction(1, 128), "FX6 maximum clamps");
    require(!NdStateMachine::step(transmission, UINT64_MAX, 1, format), "FX6 sentinel is rejected");

    std::cout << "nd_state_machine_test passed" << std::endl;
    return 0;
}
