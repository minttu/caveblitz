#include "Explosion.h"

Explosion::Explosion(uint16_t x, uint16_t y, uint8_t size) : x(x), y(y), size(size) {
}

void Explosion::draw(SDL2pp::Renderer *renderer, float dt) {
    this->ttl -= dt;
    if (this->ttl < 0 || this->destroy) {
        this->destroy = true;
        return;
    }

    int offset = size == 1 ? 0 : -1;
    for (int i = 0; i <= size - 1; i++) {
        offset += i + 1;
    }

    SDL2pp::Rect src_rect(offset, 0, size, size);

    SDL2pp::Rect dest_rect(x - (size / 2), y - (size / 2), size, size);

    this->texture->SetBlendMode(SDL_BLENDMODE_BLEND);
    this->texture->SetAlphaMod(
            static_cast<Uint8>(255 * std::log2(1 + (this->ttl / this->max_ttl))));
    renderer->Copy(*this->texture, src_rect, dest_rect);
}