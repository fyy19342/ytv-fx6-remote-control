#include "nd_controller.h"
#include "nd_state_machine.h"

bool NdController::apply(NdProperty property, uint64_t value, std::string& error) {
    const auto before = read_(property);
    if (before.readable && before.value == value) return true;
    if (!before.writable) {
        error = "ND property is unavailable or not writable.";
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
        error += " ND ON aborted; ND OFF confirmed. Manual/Step settings may remain.";
    } else {
        error += " ND OFF rollback FAILED; ND state is unconfirmed. Check the camera immediately. " + rollback_error;
    }
    return false;
}

bool NdController::set(bool enabled, std::string& error) {
    error.clear();
    if (!enabled) return apply(NdProperty::Filter, values_.off, error);
    // Sequential SDK calls cannot be atomic. Any failure fails closed to ND OFF,
    // including when b was pressed while ND was already ON.
    if (!apply(NdProperty::Mode, values_.manual, error) ||
        !apply(NdProperty::Switching, values_.step, error) ||
        !apply(NdProperty::Filter, values_.on, error)) {
        return fail_off(error, error);
    }
    const auto density = read_(NdProperty::Density);
    const auto minimum = NdStateMachine::brightest(density.possible);
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
    const auto density = read_(NdProperty::Density);
    if (!density.readable || !density.writable) {
        error = "ND density is unavailable or not writable.";
        return false;
    }
    const auto target = NdStateMachine::step(density.possible, density.value, direction);
    if (!target) {
        error = "ND density has no valid current value or steps.";
        return false;
    }
    return apply(NdProperty::Density, *target, error);
}
