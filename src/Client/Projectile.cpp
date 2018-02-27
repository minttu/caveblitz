#include "Projectile.h"

Projectile::Projectile(uint8_t projectile_type)
        : projectile_type(gsl::at(PROJECTILE_TYPES, projectile_type)) {
    this->x = 0.0f;
    this->y = 0.0f;
    this->rotation = 0.0f;
}

void Projectile::draw(SDL2pp::Renderer *renderer) const {
    SDL2pp::Rect dot_src_rect = this->projectile_type.texture_bounds;
    const int dot_size = dot_src_rect.GetW();
    SDL2pp::Rect dot_dst_rect(static_cast<int>(roundf(this->x) - (dot_size - 2) / 2),
                              static_cast<int>(roundf(this->y) - (dot_size - 2) / 2),
                              dot_size,
                              dot_size);

    renderer->Copy(*this->texture, dot_src_rect, dot_dst_rect, this->rotation);
}
