#include "nd_controller.h"
#include "nd_state_machine.h"
#include <algorithm>

bool NdController::prepare(bool manual, std::string& error) {
    const auto switching = read_(NdProperty::Switching);
    const auto available = [&](uint64_t value) {
        return (switching.readable && switching.value == value) ||
            std::find(switching.possible.begin(), switching.possible.end(), value) != switching.possible.end();
    };
    // FX6 exposes Preset/Variable, not Step. "One step" describes our movement
    // through supported density values; it does not require the SDK Step mode.
    uint64_t target;
    if (available(values_.variable)) target = values_.variable;
    else if (available(values_.step)) target = values_.step;
    else {
        error = "Camera does not expose a controllable Variable/Step ND mode.";
        return false;
    }
    if (!apply(NdProperty::Switching, target, error)) return false;
    return !manual || apply(NdProperty::Mode, values_.manual, error);
}

bool NdController::apply(NdProperty property, uint64_t value, std::string& error) {
    const auto before = read_(property);
    if (before.readable && before.value == value) return true;
    if (!before.writable) {
        error = "ND property is unavailable or not writable.";
        return false;
    }
    if (!before.possible.empty() &&
            std::find(before.possible.begin(), before.possible.end(), value) == before.possible.end()) {
        error = "Requested ND value is not in the camera's available values.";
        return false;
    }
    if (!write_(property, value, error)) return false;
    const auto after = read_(property);
    if (!after.readable || after.value != value) {
        error = "ND property readback did not confirm the requested value.";
        return false;
    }
    return true;
}

bool NdController::fail_off(const std::string& cause, std::string& error) {
    // A failed SDK write/timeout may already have changed the camera.
    // Request OFF even when cached state still reports OFF, then verify it.
    std::string rollback_error;
    const bool written = write_(NdProperty::Filter, values_.off, rollback_error);
    const auto after = read_(NdProperty::Filter);
    error = cause;
    if (written && after.readable && after.value == values_.off) {
        error += " ND ON aborted; ND OFF confirmed. Manual/Variable settings may remain.";
    } else {
        error += " ND OFF rollback FAILED; ND state is unconfirmed. Check the camera immediately. " + rollback_error;
    }
    return false;
}

bool NdController::set(bool enabled, std::string& error) {
    error.clear();
    if (!enabled) {
        const auto filter = read_(NdProperty::Filter);
        if (filter.readable && filter.value == values_.off) return true;
        return prepare(false, error) && apply(NdProperty::Filter, values_.off, error);
    }
    // Sequential SDK calls cannot be atomic. Any failure fails closed to ND OFF,
    // including when b was pressed while ND was already ON.
    if (!prepare(true, error) ||
        !apply(NdProperty::Filter, values_.on, error)) {
        return fail_off(error, error);
    }
    const auto density = read_(NdProperty::Density);
    const auto minimum = NdStateMachine::brightest(density.possible, values_.format);
    if (!density.readable || !density.writable || !minimum) {
        return fail_off("Camera did not expose a writable minimum ND density.", error);
    }
    if (!apply(NdProperty::Density, *minimum, error)) return fail_off(error, error);
    const auto filter = read_(NdProperty::Filter);
    if (!filter.readable || filter.value != values_.on) {
        return fail_off("ND ON was not confirmed after setting density.", error);
    }
    return true;
}

bool NdController::step(int direction, std::string& error) {
    error.clear();
    const auto filter = read_(NdProperty::Filter);
    if (!filter.readable || filter.value != values_.on) {
        error = "ND is OFF or unavailable. Press b to turn it ON at the brightest value.";
        return false;
    }
    if (direction == 0) return true;
    if (!prepare(true, error)) return false;
    const auto prepared_filter = read_(NdProperty::Filter);
    if (!prepared_filter.readable || prepared_filter.value != values_.on) {
        error = "ND became OFF while preparing manual control; no density was changed.";
        return false;
    }
    const auto density = read_(NdProperty::Density);
    if (!density.readable || !density.writable) {
        error = "ND density is unavailable or not writable.";
        return false;
    }
    const auto target = NdStateMachine::step(density.possible, density.value, direction, values_.format);
    if (!target) {
        error = "ND density has no valid current value or steps.";
        return false;
    }
    return apply(NdProperty::Density, *target, error);
}
