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
    size_t last_index{0};
    DeltaTime last[15];

    FPSManager();

    DeltaTime delay();

    float fps() const;
};

#endif // CAVEBLITZ_FPSMANAGER_H
