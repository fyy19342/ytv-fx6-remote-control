#include "network_target.h"
#include <cassert>

int main() {
    const auto a = parse_network_target("192.168.0.5");
    assert(a && a->sdk_ip == 0x0500a8c0u);
    assert((a->object_id == std::array<uint8_t,6>{2,0,192,168,0,5}));
    const auto b = parse_network_target("192.168.0.6");
    assert(b && a->object_id != b->object_id);
    assert(parse_network_target("169.254.10.11"));
    for (const auto* invalid : {"", "camera.local", "192.168.0", "192.168.0.256", "192.168.0.-1",
            "192.168.0.01", "192.168.0.5:22", " 192.168.0.5", "192.168.0.5 ", "192.168..5",
            "192.168.0.5.6", "127.0.0.1", "0.0.0.0", "224.0.0.1", "255.255.255.255"}) {
        assert(!parse_network_target(invalid));
    }
}
