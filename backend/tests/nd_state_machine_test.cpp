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

    std::cout << "nd_state_machine_test passed" << std::endl;
    return 0;
}
