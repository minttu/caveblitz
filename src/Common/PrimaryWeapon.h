#ifndef CAVEBLITZ_COMMON_PRIMARY_WEAPON_H
#define CAVEBLITZ_COMMON_PRIMARY_WEAPON_H

#include <SDL2pp/Rect.hh>

struct PrimaryWeapon {
    SDL2pp::Rect texture_bounds;
    uint8_t damage;
    uint8_t explosion_size;
    float cooldown;
    float velocity;

    PrimaryWeapon(SDL2pp::Rect texture_bounds,
                  uint8_t damage,
                  uint8_t explosion_size,
                  float cooldown,
                  float velocity)
            : texture_bounds(texture_bounds),
              damage(damage),
              explosion_size(explosion_size),
              cooldown(cooldown),
              velocity(velocity) {
    }
};
using PrimaryWeapon = struct PrimaryWeapon;

const static PrimaryWeapon PRIMARY_WEAPONS[] = {
        PrimaryWeapon(SDL2pp::Rect(0, 0, 4, 4), 1, 2, 0.20f, 300.0f),
        PrimaryWeapon(SDL2pp::Rect(4, 0, 4, 4), 2, 4, 0.25f, 220.0f),
        PrimaryWeapon(SDL2pp::Rect(8, 0, 6, 6), 3, 4, 0.3f, 200.0f),
        PrimaryWeapon(SDL2pp::Rect(14, 0, 6, 6), 5, 6, 0.4f, 180.0f),
        PrimaryWeapon(SDL2pp::Rect(20, 0, 6, 6), 7, 8, 0.5f, 160.0f)};

#endif // CAVEBLITZ_COMMON_PRIMARY_WEAPON_H
