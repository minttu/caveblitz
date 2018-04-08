#include "Ship.h"

Ship::Ship(uint8_t player_id) {
    this->player_id = player_id;
    this->color = gsl::at(SHIP_COLORS, player_id);
}

void Ship::draw(SDL2pp::Renderer *renderer, float dt) {
    if (this->health == 0) {
        return;
    }

    SDL2pp::Rect src_rect(1, 1, 28, 28);
    SDL2pp::Rect rect(static_cast<int>(roundf(this->x) - 14),
                      static_cast<int>(roundf(this->y) - 14),
                      28,
                      28);

    this->texture->SetBlendMode(SDL_BLENDMODE_BLEND);

    if (this->taken_damage_counter > 0.0f) {
        this->taken_damage_counter -= dt * SHIP_TAKE_DAMAGE_FADE;
        this->texture->SetColorMod(
                static_cast<Uint8>(this->color.r +
                                   (255 - this->color.r) * this->taken_damage_counter),
                static_cast<Uint8>(this->color.g +
                                   (255 - this->color.g) * this->taken_damage_counter),
                static_cast<Uint8>(this->color.b +
                                   (255 - this->color.b) * this->taken_damage_counter));
    } else {
        this->texture->SetColorMod(this->color.r, this->color.g, this->color.b);
    }
    renderer->Copy(*this->texture, src_rect, rect, this->rotation, SDL2pp::NullOpt, SDL_FLIP_NONE);

    const int dot_size = 2;
    SDL2pp::Rect dot_src_rect(14, 14, dot_size, dot_size);
    SDL2pp::Rect dot_dst_rect(static_cast<int>(roundf(this->x) - 1),
                              static_cast<int>(roundf(this->y) - 1),
                              dot_size,
                              dot_size);

    this->texture->SetColorMod(255, 255, 255);
    renderer->Copy(*this->texture, dot_src_rect, dot_dst_rect);
}

void Ship::damage_taken() {
    this->taken_damage_counter = SHIP_TAKE_DAMAGE_TIME;
}
