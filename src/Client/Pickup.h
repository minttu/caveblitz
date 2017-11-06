#ifndef CAVEBLITZ_CLIENT_PICKUP_H
#define CAVEBLITZ_CLIENT_PICKUP_H

#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>

class Pickup {
public:
    std::shared_ptr<SDL2pp::Texture> texture;
    float x;
    float y;

    Pickup();

    void draw(SDL2pp::Renderer *renderer) const;
};

#endif // CAVEBLITZ_CLIENT_PICKUP_H
