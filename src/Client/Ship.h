#ifndef CAVEBLITZ_SHIP_H
#define CAVEBLITZ_SHIP_H

#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>

class Ship {
public:
    std::shared_ptr<SDL2pp::Texture> texture;
    float x;
    float y;
    float rotation;
    SDL2pp::Color color;

    Ship();

    void draw(SDL2pp::Renderer *renderer, double dt) const;
};

#endif // CAVEBLITZ_SHIP_H
