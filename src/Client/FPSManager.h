#ifndef CAVEBLITZ_FPSMANAGER_H
#define CAVEBLITZ_FPSMANAGER_H

#include <cstdint>

using DeltaTime = float;

class FPSManager {
public:
    uint32_t frame_count;
    float rate_ticks;
    uint32_t base_ticks;
    uint32_t last_ticks;
    uint32_t rate;

    FPSManager();

    DeltaTime delay();
};

#endif // CAVEBLITZ_FPSMANAGER_H
