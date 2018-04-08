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

const static float SHIP_TAKE_DAMAGE_TIME = 0.75;
const static float SHIP_TAKE_DAMAGE_FADE = 3.0f;

class Ship {
public:
    std::shared_ptr<SDL2pp::Texture> texture;
    uint8_t player_id;
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
    SDL2pp::Color color;
    int health = 100;
    bool ready_to_play = false;
    float taken_damage_counter = 0.0f;

    explicit Ship(uint8_t player_id);

    void draw(SDL2pp::Renderer *renderer, float dt);

    void damage_taken();
};

#endif // CAVEBLITZ_SHIP_H
