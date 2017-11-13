#ifndef CAVEBLITZ_CLIENT_EXPLOSION_H
#define CAVEBLITZ_CLIENT_EXPLOSION_H

#include <cmath>
#include <cstdint>
#include <memory>

#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>

class Explosion {
public:
    uint16_t x;
    uint16_t y;
    uint8_t size;
    std::shared_ptr<SDL2pp::Texture> texture;
    float max_ttl{0.25f};
    float ttl{max_ttl};
    bool destroy{false};

    Explosion(uint16_t x, uint16_t y, uint8_t size);
    void draw(SDL2pp::Renderer *renderer, float dt);
};

#endif // CAVEBLITZ_CLIENT_EXPLOSION_H
