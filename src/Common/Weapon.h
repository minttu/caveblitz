#ifndef CAVEBLITZ_COMMON_PRIMARY_WEAPON_H
#define CAVEBLITZ_COMMON_PRIMARY_WEAPON_H

#include <SDL2pp/Rect.hh>

#include "ProjectileType.h"

struct Weapon {
    ProjectileType projectile_type;
    float cooldown;
    float velocity;

    Weapon(ProjectileType projectile_type, float cooldown, float velocity)
            : projectile_type(projectile_type), cooldown(cooldown), velocity(velocity) {
    }
};
using Weapon = struct Weapon;

const static Weapon WEAPONS[] = {Weapon(PROJECTILE_TYPES[0], 0.20f, 300.0f),
                                 Weapon(PROJECTILE_TYPES[1], 0.25f, 220.0f),
                                 Weapon(PROJECTILE_TYPES[2], 0.3f, 200.0f),
                                 Weapon(PROJECTILE_TYPES[3], 0.4f, 180.0f),
                                 Weapon(PROJECTILE_TYPES[4], 0.5f, 160.0f)};

#endif // CAVEBLITZ_COMMON_PRIMARY_WEAPON_H
