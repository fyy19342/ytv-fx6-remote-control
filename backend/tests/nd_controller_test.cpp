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
            }, {0, 1, 1, 1});
    }
};

int main() {
    std::string error;
    Camera normal;
    auto c = normal.control();
    check(!c.step(1, error) && normal.writes == 0, "OFF step must not write any property");
    check(!c.step(-1, error) && normal.writes == 0, "OFF brighten must not write");
    check(c.set(true, error), "b succeeds");
    check(normal.state[NdProperty::Density].value == 60, "b selects minimum");
    check(c.step(1, error) && normal.state[NdProperty::Density].value == 90, "d darkens one step");
    check(c.step(-1, error) && normal.state[NdProperty::Density].value == 60, "u brightens one step");
    check(c.step(1, error) && c.set(true, error) && normal.state[NdProperty::Density].value == 60,
          "b resets an already-ON ND to minimum");
    check(c.set(false, error) && normal.state[NdProperty::Filter].value == 0, "m turns OFF");
    check(c.set(true, error) && normal.state[NdProperty::Density].value == 60, "ON never restores previous density");
    for (int failure = 1; failure <= 4; ++failure) {
        Camera broken;
        broken.fail_at = failure;
        check(!broken.control().set(true, error), "ON stage failure is reported");
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
          "failed b while ON follows explicit OFF policy");
    std::cout << "nd_controller_test passed (including injected SDK failures)\n";
}
