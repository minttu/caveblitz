#include "Projectile.h"

Projectile::Projectile(uint8_t projectile_type)
        : projectile_type(gsl::at(PROJECTILE_TYPES, projectile_type)) {
    this->x = 0.0f;
    this->y = 0.0f;
    this->rotation = 0.0f;
}

void Projectile::draw(SDL2pp::Renderer *renderer) const {
    SDL2pp::Rect src_rect = this->projectile_type.texture_bounds;
    const int projectile_size = src_rect.GetW();
    SDL2pp::Rect destination_rect(static_cast<int>(roundf(this->x) - projectile_size / 2.0f),
                                  static_cast<int>(roundf(this->y) - projectile_size / 2.0f),
                                  projectile_size,
                                  projectile_size);

    renderer->Copy(*this->texture, src_rect, destination_rect, this->rotation);
}
