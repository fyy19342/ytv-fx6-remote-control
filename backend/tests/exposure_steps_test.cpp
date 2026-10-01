#include "exposure_steps.h"
#include <cstdlib>
#include <iostream>
void check(bool ok) { if (!ok) std::exit(1); }
int main() {
    const std::vector<uint64_t> iris{400, 280, 560, 0xfffd, 0xfffe, 0xffff, 0};
    check(exposure_step(ExposureKind::Iris, iris, 400, -1) == 280);
    check(exposure_step(ExposureKind::Iris, iris, 400, 1) == 560);
    check(exposure_step(ExposureKind::Iris, iris, 0xfffd, -1) == 560);
    check(!exposure_step(ExposureKind::Iris, iris, 0xffff, -1));
    const std::vector<uint64_t> iso{800, 1600, 0x10000064, 0xffffff, 0x1fffffff, 0};
    check(exposure_step(ExposureKind::Iso, iso, 800, 1) == 1600);
    check(exposure_step(ExposureKind::Iso, iso, 800, -1) == 0x10000064);
    check(exposure_step(ExposureKind::Iso, iso, 1600, 1) == 1600);
    check(!exposure_step(ExposureKind::Iso, iso, 0xffffff, 1));
    check(exposure_number(ExposureKind::Iso, 0x10000064) == 100);
    std::cout << "exposure_steps_test passed\n";
}
