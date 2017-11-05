#include <SDL2/SDL_timer.h>

#include "FPSManager.h"

FPSManager::FPSManager() {
    this->frame_count = 0;
    this->rate = 60;
    this->rate_ticks = 1000.0f / static_cast<float>(this->rate);
    this->base_ticks = SDL_GetTicks();
    this->last_ticks = this->base_ticks;
    for (float &i : this->last) {
        i = 0;
    }
}

DeltaTime FPSManager::delay() {
    this->frame_count += 1;

    uint32_t current_ticks = SDL_GetTicks();
    DeltaTime time_passed = static_cast<float>(current_ticks - this->last_ticks) / 1000.0f;
    this->last_ticks = current_ticks;
    uint32_t target_ticks =
            this->base_ticks + static_cast<uint32_t>(this->frame_count * this->rate_ticks);

    if (current_ticks <= target_ticks) {
        SDL_Delay(target_ticks - current_ticks);
    } else {
        this->frame_count = 0;
        this->base_ticks = SDL_GetTicks();
    }

    if (this->last_index == (sizeof(this->last) / sizeof(this->last[0]))) {
        this->last_index = 0;
    }
    this->last[this->last_index++] = time_passed;

    return time_passed;
}

float FPSManager::fps() const {
    float sum = 0;
    for (float i : this->last) {
        sum += i;
    }

    return 1.0f / (sum / static_cast<float>(sizeof(this->last) / sizeof(this->last[0])));
}
