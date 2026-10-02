#include "awb_controller.h"
#include <cassert>
#include <iostream>
#include <map>

int main() {
    std::map<AwbProperty, AwbReading> state{
        {AwbProperty::Mode, {true, true, 1, {1, 2}}},
        {AwbProperty::Button, {true, true, 1, {1, 2}}}};
    std::vector<std::pair<AwbProperty, uint64_t>> writes;
    int holds = 0;
    bool fail_down = false, fail_up = false, ignore_mode = false;
    AwbController controller([&](auto p) { return state[p]; },
        [&](auto p, auto v, std::string& error) {
            writes.emplace_back(p, v);
            if (!(p == AwbProperty::Mode && ignore_mode)) state[p].value = v;
            if (p == AwbProperty::Button && ((v == 2 && fail_down) || (v == 1 && fail_up))) {
                error = "injected failure after physical write"; return false;
            }
            return true;
        }, [&] { ++holds; }, 2, 1, 2);
    std::string error;
    assert(controller.trigger(error) && holds == 1 && writes.size() == 3);
    assert(writes[0].first == AwbProperty::Mode && writes[0].second == 2);
    assert(writes[1].second == 2 && writes[2].second == 1);
    assert(state[AwbProperty::Mode].value == 2 && state[AwbProperty::Button].value == 1);
    writes.clear();
    assert(controller.trigger(error) && writes.size() == 2 && holds == 2);
    fail_down = true;
    writes.clear();
    assert(!controller.trigger(error) && writes.size() == 2 && holds == 2);
    assert(state[AwbProperty::Button].value == 1 && error.find("release was sent") != std::string::npos);
    fail_down = false; fail_up = true;
    assert(!controller.trigger(error) && error.find("release could not") != std::string::npos);
    fail_up = false;
    state[AwbProperty::Mode].value = 1; ignore_mode = true;
    writes.clear();
    assert(!controller.trigger(error) && writes.size() == 1 && error.find("not confirmed") != std::string::npos);
    ignore_mode = false;
    state[AwbProperty::Mode].writable = false;
    writes.clear();
    assert(!controller.trigger(error) && writes.empty());
    state[AwbProperty::Mode].value = 2; // Already manual: readonly mode is fine.
    state[AwbProperty::Button].writable = false;
    assert(!controller.trigger(error) && writes.empty());
    state[AwbProperty::Button].writable = true;
    state[AwbProperty::Button].value = 2;
    assert(!controller.trigger(error) && writes.empty());
    state[AwbProperty::Button].value = 1;
    state[AwbProperty::Button].possible = {2}; // Never press without release capability.
    assert(!controller.trigger(error) && writes.empty());

    AwbProgress progress;
    progress.notify(true, "old notification");
    assert(progress.status == "idle");
    assert(progress.begin(0) && !progress.begin(100));
    assert(progress.retry_after_ms(0) == 3000);
    assert(progress.retry_after_ms(2999) == 1);
    progress.notify(false, "white area too small");
    assert(progress.status == "failed");
    assert(!progress.begin(2999)); // Failure does not release the cooldown early.
    assert(progress.begin(3000));
    progress.notify(true, "SDK confirmed AWB");
    assert(progress.status == "completed");
    assert(!progress.begin(5999)); // Nor does an early completion.
    assert(progress.begin(6000));
    progress.expire(9000);
    assert(progress.status == "running" && progress.retry_after_ms(9000) == 0);
    progress.expire(20999);
    assert(progress.status == "running");
    progress.expire(21000);
    assert(progress.status == "unconfirmed");
    progress.notify(true, "late notification");
    assert(progress.status == "unconfirmed");

    AwbProgress pending;
    assert(pending.begin(0));
    assert(!pending.begin(2999));
    assert(pending.begin(3000)); // No result notification is required to retry.
    assert(pending.status == "unconfirmed" && pending.result_ambiguous);
    pending.notify(true, "old request completed after new request started");
    assert(pending.status == "unconfirmed");
    assert(!pending.begin(5999) && pending.begin(6000));
    pending.fail("write failure");
    assert(!pending.begin(8999) && pending.begin(9000));
    pending.notify(false, "ambiguous old failure");
    assert(pending.status == "unconfirmed");
    assert(progress.begin(22000)); // Also keep late results ambiguous after timeout.
    progress.notify(true, "old request result");
    assert(progress.status == "unconfirmed");

    AwbProgress disconnected;
    assert(disconnected.begin(0));
    disconnected.disconnect();
    assert(disconnected.status == "unconfirmed");
    assert(!disconnected.begin(2999) && disconnected.begin(3000));
    std::cout << "awb_controller_test passed\n";
}
