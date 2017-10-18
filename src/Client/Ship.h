#ifndef CAVEBLITZ_SHIP_H
#define CAVEBLITZ_SHIP_H

#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>

#include <gsl/gsl>

const static SDL2pp::Color SHIP_COLORS[8] = {SDL2pp::Color(25, 25, 245),
                                             SDL2pp::Color(210, 40, 40),
                                             SDL2pp::Color(215, 215, 30),
                                             SDL2pp::Color(150, 15, 250),
                                             SDL2pp::Color(15, 210, 80),
                                             SDL2pp::Color(255, 150, 5),
                                             SDL2pp::Color(150, 255, 240),
                                             SDL2pp::Color(255, 190, 255)};

class Ship {
public:
    std::shared_ptr<SDL2pp::Texture> texture;
    uint8_t player_id;
    float x;
    float y;
    float rotation;
    SDL2pp::Color color;

    explicit Ship(uint8_t player_id);

    void draw(SDL2pp::Renderer *renderer, float dt) const;
};

#endif // CAVEBLITZ_SHIP_H
