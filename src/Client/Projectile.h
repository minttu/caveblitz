#ifndef CAVEBLITZ_CLIENT_PROJECTILE_H
#define CAVEBLITZ_CLIENT_PROJECTILE_H

#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>

#include <gsl/gsl>

#include "../Common/PrimaryWeapon.h"

class Projectile {
public:
    std::shared_ptr<SDL2pp::Texture> texture;
    float x;
    float y;
    PrimaryWeapon primary_weapon;
    uint8_t weapon_type;

    explicit Projectile(uint8_t weapon_type);

    void draw(SDL2pp::Renderer *renderer) const;
};

#endif // CAVEBLITZ_CLIENT_PROJECTILE_H
