#include "nd_controller.h"
#include <cstdlib>
#include <iostream>
#include <map>

void check(bool ok, const char* message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

struct Camera {
    std::map<NdProperty, NdReading> state = {
        {NdProperty::Filter, {true, true, 0, {0, 1}}},
        {NdProperty::Mode, {true, true, 0, {0, 1}}},
        {NdProperty::Switching, {true, true, 0, {0, 1}}},
        {NdProperty::Density, {true, true, 210, {210, 120, 60, 90, 0xffff, 0}}}
    };
    int writes = 0;
    int fail_at = 0;
    bool fail_off = false;
    bool ignore_density = false;
    NdController control() {
        return NdController([this](auto p) { return state[p]; },
            [this](auto p, auto v, std::string& error) {
                ++writes;
                if (fail_off && p == NdProperty::Filter && v == 0) {
                    error = "injected rollback failure"; return false;
                }
                if (!(ignore_density && p == NdProperty::Density)) state[p].value = v;
                if (writes == fail_at) { error = "injected failure after physical write"; return false; }
                return true;
            }, {0, 1, 1, 1, 2});
    }
};

int main() {
    std::string error;
    Camera normal;
    auto c = normal.control();
    check(!c.step(1, error) && normal.writes == 0, "OFF step must not write any property");
    check(!c.step(-1, error) && normal.writes == 0, "OFF brighten must not write");
    check(c.toggle(error), "b from OFF succeeds");
    check(normal.state[NdProperty::Density].value == 60, "b from OFF selects minimum");
    check(c.step(1, error) && normal.state[NdProperty::Density].value == 90, "d darkens one step");
    check(c.step(-1, error) && normal.state[NdProperty::Density].value == 60, "u brightens one step");
    check(c.step(1, error) && c.toggle(error) && normal.state[NdProperty::Filter].value == 0,
          "b while ON turns OFF");
    check(c.toggle(error) && normal.state[NdProperty::Density].value == 60,
          "next b returns ON at minimum, not the previous density");
    check(c.set(false, error) && normal.state[NdProperty::Filter].value == 0, "m turns OFF");
    check(c.set(true, error) && normal.state[NdProperty::Density].value == 60, "ON never restores previous density");
    // Body changes are read for every toggle, without a frontend cache.
    normal.state[NdProperty::Filter].value = 0;
    check(c.toggle(error) && normal.state[NdProperty::Filter].value == 1, "toggle uses latest body state");
    for (const auto readable : {false, true}) {
        Camera unknown;
        unknown.state[NdProperty::Filter] = {readable, true, 99, {0, 1}};
        check(!unknown.control().toggle(error) && unknown.writes == 0, "unknown toggle state never writes");
    }
    for (int failure = 1; failure <= 4; ++failure) {
        Camera broken;
        broken.fail_at = failure;
        check(!broken.control().toggle(error), "toggle ON stage failure is reported");
        check(broken.state[NdProperty::Filter].value == 0, "each ON stage failure returns OFF");
        check(error.find("OFF confirmed") != std::string::npos, "rollback confirmation included");
    }
    Camera missing;
    missing.state[NdProperty::Density].possible = {0, 0xffff};
    check(!missing.control().set(true, error) && missing.state[NdProperty::Filter].value == 0,
          "missing minimum rolls back OFF");
    Camera unreadable;
    unreadable.state[NdProperty::Density].readable = false;
    check(!unreadable.control().set(true, error) && unreadable.state[NdProperty::Filter].value == 0,
          "density read failure rolls back OFF");
    Camera mismatch;
    mismatch.ignore_density = true;
    check(!mismatch.control().set(true, error) && mismatch.state[NdProperty::Filter].value == 0,
          "false successful write detected by readback");
    Camera rollback;
    rollback.fail_at = 4;
    rollback.fail_off = true;
    check(!rollback.control().set(true, error), "rollback failure reports error");
    check(error.find("rollback FAILED") != std::string::npos, "rollback failure is never claimed OFF");
    Camera already_on;
    already_on.state[NdProperty::Filter].value = 1;
    already_on.state[NdProperty::Density].writable = false;
    check(!already_on.control().set(true, error) && already_on.state[NdProperty::Filter].value == 0,
          "failed explicit ON follows OFF recovery policy");
    // Actual FX6 readback: Preset mode permits switching to Variable, while
    // Filter and Density remain read-only until that switch has completed.
    for (const auto action : {0, 1, 2}) {
        Camera fx6;
        fx6.state[NdProperty::Filter] = {true, false, 1, {}};
        fx6.state[NdProperty::Mode] = {true, false, 1, {0, 1}};
        fx6.state[NdProperty::Switching] = {true, true, 0, {0, 2}};
        fx6.state[NdProperty::Density] = {true, false, 90, {}};
        bool unsupported_write = false;
        NdController controller([&](auto p) { return fx6.state[p]; },
            [&](auto p, auto value, std::string&) {
                ++fx6.writes;
                if (p == NdProperty::Switching) {
                    unsupported_write |= value != 2;
                    fx6.state[NdProperty::Filter].writable = true;
                    fx6.state[NdProperty::Filter].possible = {0, 1};
                    fx6.state[NdProperty::Density].writable = true;
                    fx6.state[NdProperty::Density].possible = {60, 90, 120};
                }
                fx6.state[p].value = value;
                return true;
            }, {0, 1, 1, 1, 2});
        check(action == 0 ? controller.set(false, error) :
              action == 1 ? controller.set(true, error) : controller.step(1, error),
              "FX6 Preset prepares Variable before OFF/ON/step");
        check(!unsupported_write, "FX6 never receives unsupported Step mode");
        check(fx6.state[NdProperty::Switching].value == 2, "FX6 Variable readback");
        check(action == 0 ? fx6.state[NdProperty::Filter].value == 0 :
              fx6.state[NdProperty::Density].value == (action == 1 ? 60 : 120),
              "FX6 command reaches requested value");
    }
    Camera transmittance;
    const uint64_t nd4 = (1ULL << 32) | 4;
    const uint64_t nd48 = (10ULL << 32) | 48;
    const uint64_t nd8 = (1ULL << 32) | 8;
    transmittance.state[NdProperty::Switching] = {true, true, 2, {0, 2}};
    transmittance.state[NdProperty::Density] = {true, true, nd8, {nd8, nd48, nd4}};
    NdController tx([&](auto p) { return transmittance.state[p]; },
        [&](auto p, auto value, std::string&) { transmittance.state[p].value = value; return true; },
        {0, 1, 1, 1, 2, NdValueFormat::Transmittance});
    check(tx.set(true, error) && transmittance.state[NdProperty::Density].value == nd4, "FX6 ON uses transmittance minimum");
    check(tx.step(1, error) && transmittance.state[NdProperty::Density].value == nd48, "FX6 d writes exact fraction");
    check(tx.step(-1, error) && transmittance.state[NdProperty::Density].value == nd4, "FX6 u restores exact fraction");
    check(tx.set(false, error) && !tx.step(1, error), "FX6 OFF remains OFF on step");
    std::cout << "nd_controller_test passed (including injected SDK failures)\n";
}
