#pragma once

#include <functional>
#include <string>
#include <vector>
#include <cstdint>
#include "nd_state_machine.h"

enum class NdProperty { Filter, Mode, Switching, Density };

struct NdReading {
    bool readable = false;
    bool writable = false;
    uint64_t value = 0;
    std::vector<uint64_t> possible;
};

// SDK access is injected so failures after a physical write can be tested.
class NdController {
public:
    struct Values {
        uint64_t off, on, manual, step, variable;
        NdValueFormat format = NdValueFormat::OpticalDensity;
    };
    using Read = std::function<NdReading(NdProperty)>;
    using Write = std::function<bool(NdProperty, uint64_t, std::string&)>;
    NdController(Read read, Write write, Values values)
        : read_(read), write_(write), values_(values) {}
    bool set(bool enabled, std::string& error);
    bool step(int direction, std::string& error);

private:
    bool apply(NdProperty property, uint64_t value, std::string& error);
    bool prepare(bool manual, std::string& error);
    bool fail_off(const std::string& cause, std::string& error);
    Read read_;
    Write write_;
    Values values_;
};
