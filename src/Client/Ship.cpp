#include "Ship.h"

Ship::Ship() {
    this->x = 0.0f;
    this->y = 0.0f;
    this->rotation = 0.0f;
    this->color = SDL2pp::Color(255 / 4 * 3, 0, 0);
}

void Ship::draw(SDL2pp::Renderer *renderer, double /*unused*/) const {
    SDL2pp::Rect src_rect(1, 1, 32, 32);
    SDL2pp::Rect rect(static_cast<int>(roundf(this->x) - 16),
                      static_cast<int>(roundf(this->y) - 16),
                      32,
                      32);

    this->texture->SetBlendMode(SDL_BLENDMODE_BLEND);
    this->texture->SetColorMod(this->color.r, this->color.g, this->color.b);
    renderer->Copy(*this->texture, src_rect, rect, this->rotation, SDL2pp::NullOpt, SDL_FLIP_NONE);

    const int dot_size = 2;
    SDL2pp::Rect dot_src_rect(16, 16, dot_size, dot_size);
    SDL2pp::Rect dot_dst_rect(static_cast<int>(roundf(this->x) - 1),
                              static_cast<int>(roundf(this->y) - 1),
                              dot_size,
                              dot_size);

    this->texture->SetColorMod(255, 255, 255);
    renderer->Copy(*this->texture, dot_src_rect, dot_dst_rect);
}
