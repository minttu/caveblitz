#include "Pickup.h"

Pickup::Pickup() {
    this->x = 0.0f;
    this->y = 0.0f;
}

void Pickup::draw(SDL2pp::Renderer *renderer) const {
    SDL2pp::Rect dot_src_rect(1, 1, 16, 16);
    SDL2pp::Rect dot_dst_rect(static_cast<int>(roundf(this->x) - 8),
                              static_cast<int>(roundf(this->y) - 8),
                              16,
                              16);

    renderer->Copy(*this->texture, dot_src_rect, dot_dst_rect);
}
