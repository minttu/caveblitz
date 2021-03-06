#ifndef CAVEBLITZ_COMMON_PROJECTILE_TYPE_H
#define CAVEBLITZ_COMMON_PROJECTILE_TYPE_H

#include <SDL2pp/Rect.hh>

struct ProjectileType {
    SDL2pp::Rect texture_bounds;
    uint8_t damage;
    uint8_t explosion_size;

    ProjectileType(SDL2pp::Rect texture_bounds, uint8_t damage, uint8_t explosion_size)
            : texture_bounds(texture_bounds), damage(damage), explosion_size(explosion_size) {
    }
};
using ProjectileType = struct ProjectileType;

enum PROJECTILE_TYPE_NAMES {
    PELLET_1_PROJECTILE,
    PELLET_2_PROJECTILE,
    PELLET_3_PROJECTILE,
    PELLET_4_PROJECTILE,
    PELLET_5_PROJECTILE,
    BOMB_PROJECTILE,
    MORNING_STAR_PROJECTILE
};

const static ProjectileType PROJECTILE_TYPES[] = {ProjectileType(SDL2pp::Rect(0, 0, 4, 4), 1, 2),
                                                  ProjectileType(SDL2pp::Rect(4, 0, 4, 4), 2, 4),
                                                  ProjectileType(SDL2pp::Rect(8, 0, 6, 6), 3, 4),
                                                  ProjectileType(SDL2pp::Rect(14, 0, 6, 6), 5, 6),
                                                  ProjectileType(SDL2pp::Rect(20, 0, 6, 6), 7, 8),
                                                  ProjectileType(SDL2pp::Rect(26, 0, 13, 13),
                                                                 20,
                                                                 32),
                                                  ProjectileType(SDL2pp::Rect(0, 6, 8, 8), 5, 6)};

#endif // CAVEBLITZ_COMMON_PROJECTILE_TYPE_H
