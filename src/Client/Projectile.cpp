#include "Projectile.h"

Projectile::Projectile() {
    this->x = 0.0f;
    this->y = 0.0f;
}

void Projectile::draw(SDL2pp::Renderer *renderer) const {
    const int dot_size = 2;
    SDL2pp::Rect dot_src_rect(16, 16, dot_size, dot_size);
    SDL2pp::Rect dot_dst_rect(static_cast<int>(roundf(this->x) - 1),
                              static_cast<int>(roundf(this->y) - 1),
                              dot_size,
                              dot_size);

    this->texture->SetBlendMode(SDL_BLENDMODE_BLEND);
    this->texture->SetColorMod(255, 255, 0);
    renderer->Copy(*this->texture, dot_src_rect, dot_dst_rect);
}
