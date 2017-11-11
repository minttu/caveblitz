#ifndef CAVEBLITZ_COMMON_PRIMARY_WEAPON_H
#define CAVEBLITZ_COMMON_PRIMARY_WEAPON_H

#include <SDL2pp/Rect.hh>

#include "ProjectileType.h"

struct PrimaryWeapon {
    ProjectileType projectile_type;
    float cooldown;
    float velocity;

    PrimaryWeapon(ProjectileType projectile_type, float cooldown, float velocity)
            : projectile_type(projectile_type), cooldown(cooldown), velocity(velocity) {
    }
};
using PrimaryWeapon = struct PrimaryWeapon;

const static PrimaryWeapon PRIMARY_WEAPONS[] = {PrimaryWeapon(PROJECTILE_TYPES[0], 0.20f, 300.0f),
                                                PrimaryWeapon(PROJECTILE_TYPES[1], 0.25f, 220.0f),
                                                PrimaryWeapon(PROJECTILE_TYPES[2], 0.3f, 200.0f),
                                                PrimaryWeapon(PROJECTILE_TYPES[3], 0.4f, 180.0f),
                                                PrimaryWeapon(PROJECTILE_TYPES[4], 0.5f, 160.0f)};

#endif // CAVEBLITZ_COMMON_PRIMARY_WEAPON_H
