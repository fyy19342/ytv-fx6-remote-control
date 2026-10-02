#include "shutter_controller.h"
#include <cassert>
#include <iostream>
#include <map>

constexpr uint64_t fraction(uint64_t n, uint64_t d) { return (n << 32) | d; }

int main() {
    const auto s30 = fraction(1, 30), s60 = fraction(1, 60), s120 = fraction(1, 120);
    const auto fractional = fraction(1001, 60000);
    const std::vector<uint64_t> candidates{s30, 0, s120, fractional, s60, fraction(2, 120), UINT64_MAX, fraction(1, 0)};
    assert(shutter_step(candidates, s60, 1) == fractional);
    assert(shutter_step(candidates, fractional, -1) == s60);
    assert(shutter_step(candidates, s60, -1) == s120);
    assert(shutter_step(candidates, s30, 1) == s30);
    assert(shutter_step(candidates, s120, -1) == s120);
    assert(shutter_step(candidates, s60, 0) == s60);
    assert(shutter_step(candidates, fraction(1, 50), 1) == s30);
    assert(!shutter_step(candidates, 0, 1));
    assert(!shutter_step({0, UINT64_MAX}, s60, 1));
    assert(shutter_step({fraction(2, 1), fraction(1, 2), fraction(1, 1)}, fraction(1, 1), 1) == fraction(2, 1));
    assert(shutter_shorter(fraction(0xfffffffd, 0xfffffffe), fraction(0xfffffffe, 0xfffffffd)));

    std::map<ShutterProperty, ShutterReading> state{
        {ShutterProperty::Mode, {true, true, 1, {1, 2, 3, 4, 5}}},
        {ShutterProperty::Speed, {true, false, s60, {}}}};
    int writes = 0;
    bool ignore_write = false;
    bool fail_write = false;
    ShutterController control([&](auto p) { return state[p]; },
        [&](auto p, auto value, std::string& error) {
            ++writes;
            if (fail_write) { error = "injected write failure"; return false; }
            if (ignore_write) return true;
            state[p].value = value;
            if (p == ShutterProperty::Mode) state[ShutterProperty::Speed] = {true, true, s60, candidates};
            return true;
        }, 2);
    std::string error;
    assert(control.step(0, error) && writes == 0);
    assert(control.step(1, error) && writes == 2);
    assert(state[ShutterProperty::Mode].value == 2 && state[ShutterProperty::Speed].value == fractional);
    assert(control.step(-1, error) && state[ShutterProperty::Speed].value == s60);
    ignore_write = true;
    assert(!control.step(-1, error) && error.find("readback") != std::string::npos);
    ignore_write = false;
    fail_write = true;
    assert(!control.step(-1, error) && error == "injected write failure");
    fail_write = false;
    state[ShutterProperty::Mode].readable = false;
    const int before = writes;
    assert(!control.step(1, error) && writes == before);
    state[ShutterProperty::Mode] = {true, false, 4, {}};
    assert(!control.step(1, error) && writes == before);
    state[ShutterProperty::Mode] = {true, true, 3, {3, 4}};
    assert(!control.step(1, error) && writes == before);
    state[ShutterProperty::Mode] = {true, true, 2, {2}};
    state[ShutterProperty::Speed].possible.clear();
    assert(!control.step(1, error) && writes == before);
    std::cout << "shutter_controller_test passed\n";
}
