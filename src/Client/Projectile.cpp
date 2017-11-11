#include "Projectile.h"

Projectile::Projectile(uint8_t weapon_type)
        : primary_weapon(gsl::at(PRIMARY_WEAPONS, weapon_type)), weapon_type(weapon_type) {
    this->x = 0.0f;
    this->y = 0.0f;
}

void Projectile::draw(SDL2pp::Renderer *renderer) const {
    SDL2pp::Rect dot_src_rect = this->primary_weapon.texture_bounds;
    const int dot_size = dot_src_rect.GetW();
    SDL2pp::Rect dot_dst_rect(static_cast<int>(roundf(this->x) - (dot_size - 2) / 2),
                              static_cast<int>(roundf(this->y) - (dot_size - 2) / 2),
                              dot_size,
                              dot_size);

    renderer->Copy(*this->texture, dot_src_rect, dot_dst_rect);
}
