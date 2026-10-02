#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

enum class AwbProperty { Mode, Button };
struct AwbReading {
    bool readable = false;
    bool writable = false;
    uint64_t value = 0;
    std::vector<uint64_t> possible;
};

// A momentary SDK button needs an Up attempt even if Down reports an error
// after changing the physical state. It must never be treated as a toggle.
class AwbController {
public:
    using Read = std::function<AwbReading(AwbProperty)>;
    using Write = std::function<bool(AwbProperty, uint64_t, std::string&)>;
    AwbController(Read read, Write write, std::function<void()> hold,
                  uint64_t manual, uint64_t up, uint64_t down)
        : read_(read), write_(write), hold_(hold), manual_(manual), up_(up), down_(down) {}

    bool trigger(std::string& error) {
        error.clear();
        auto mode = read_(AwbProperty::Mode);
        if (!mode.readable) {
            error = "White balance mode is unavailable.";
            return false;
        }
        if (mode.value != manual_) {
            if (!mode.writable || !allows(mode, manual_) || !write_(AwbProperty::Mode, manual_, error)) {
                if (error.empty()) error = "Set white balance to Manual / memory A or B on the camera.";
                return false;
            }
            mode = read_(AwbProperty::Mode);
            if (!mode.readable || mode.value != manual_) {
                error = "Manual white balance was not confirmed. AWB was not pressed.";
                return false;
            }
        }
        const auto button = read_(AwbProperty::Button);
        if (!button.writable || !allows(button, down_) || !allows(button, up_) ||
            (button.readable && button.value == down_)) {
            error = "AWB is unavailable or busy. Select white balance memory A or B on the camera.";
            return false;
        }
        std::string down_error, up_error;
        const bool pressed = write_(AwbProperty::Button, down_, down_error);
        if (pressed) hold_();
        const bool released = write_(AwbProperty::Button, up_, up_error);
        if (!released) {
            error = "AWB release could not be confirmed. Check the camera. " + up_error;
            return false;
        }
        if (!pressed) {
            error = "AWB press failed; release was sent. " + down_error;
            return false;
        }
        return true;
    }

private:
    static bool allows(const AwbReading& r, uint64_t value) {
        return std::find(r.possible.begin(), r.possible.end(), value) != r.possible.end();
    }
    Read read_;
    Write write_;
    std::function<void()> hold_;
    uint64_t manual_, up_, down_;
};

// Protected by the service's dedicated AWB mutex; no SDK mutex in callbacks.
struct AwbProgress {
    std::string status = "idle";
    std::string message;
    int64_t deadline_ms = 0;

    bool begin(int64_t now_ms) {
        expire(now_ms);
        if (status == "running") return false;
        status = "running";
        message = "AWB の結果確認中。カメラ本体の結果も確認してください。";
        deadline_ms = now_ms + 15000;
        return true;
    }
    void fail(const std::string& reason) { status = "failed"; message = reason; }
    void notify(bool ok, const std::string& reason) {
        if (status != "running") return;
        status = ok ? "completed" : "failed";
        message = reason;
    }
    void expire(int64_t now_ms) {
        if (status == "running" && now_ms >= deadline_ms) {
            status = "unconfirmed";
            message = "AWB の結果通知を確認できません。カメラ本体の結果を確認してください。";
        }
    }
    void disconnect() {
        if (status == "running") {
            status = "unconfirmed";
            message = "AWB の結果確認前に切断されました。カメラ本体を確認してください。";
        }
    }
};
