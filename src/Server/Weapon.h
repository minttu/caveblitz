#ifndef CAVEBLITZ_SERVER_WEAPON_H
#define CAVEBLITZ_SERVER_WEAPON_H

#include <SDL2pp/Rect.hh>

#include "../Common/ProjectileType.h"
#include "ServerPlayer.h"
#include "ServerProjectile.h"

std::shared_ptr<ServerProjectile>
create_projectile(uint8_t projectile_type, float velocity, std::shared_ptr<ServerPlayer> &player);

std::vector<std::shared_ptr<ServerProjectile>>
fire_primary_weapon(uint8_t projectile_type, float velocity, std::shared_ptr<ServerPlayer> &player);

std::vector<std::shared_ptr<ServerProjectile>>
morning_star_on_hit(std::shared_ptr<ServerProjectile> &prj, std::shared_ptr<ServerPlayer> &player);

std::vector<std::shared_ptr<ServerProjectile>>
fire_morning_star(std::shared_ptr<ServerPlayer> &player);

std::vector<std::shared_ptr<ServerProjectile>> fire_bomb(std::shared_ptr<ServerPlayer> &player);

static const uint16_t PRIMARY_WEAPON = 0;

struct Weapon {
    uint16_t max_ammo;
    float cooldown;
    std::vector<std::shared_ptr<ServerProjectile>> (*fire)(std::shared_ptr<ServerPlayer> &plr);
    bool is_primary;

    Weapon(uint16_t max_ammo,
           float cooldown,
           std::vector<std::shared_ptr<ServerProjectile>> (*fire)(
                   std::shared_ptr<ServerPlayer> &plr))
            : max_ammo(max_ammo),
              cooldown(cooldown),
              fire(fire),
              is_primary(max_ammo == PRIMARY_WEAPON) {
    }
};
using Weapon = struct Weapon;

const static Weapon WEAPONS[] = {Weapon(PRIMARY_WEAPON,
                                        0.20f,
                                        [](std::shared_ptr<ServerPlayer> &plr) {
                                            return fire_primary_weapon(0, 300.0f, plr);
                                        }),
                                 Weapon(PRIMARY_WEAPON,
                                        0.25f,
                                        [](std::shared_ptr<ServerPlayer> &plr) {
                                            return fire_primary_weapon(1, 220.0f, plr);
                                        }),
                                 Weapon(PRIMARY_WEAPON,
                                        0.3f,
                                        [](std::shared_ptr<ServerPlayer> &plr) {
                                            return fire_primary_weapon(2, 200.0f, plr);
                                        }),
                                 Weapon(PRIMARY_WEAPON,
                                        0.4f,
                                        [](std::shared_ptr<ServerPlayer> &plr) {
                                            return fire_primary_weapon(3, 180.0f, plr);
                                        }),
                                 Weapon(PRIMARY_WEAPON,
                                        0.5f,
                                        [](std::shared_ptr<ServerPlayer> &plr) {
                                            return fire_primary_weapon(4, 160.0f, plr);
                                        }),
                                 Weapon(100, 1.0f, fire_morning_star),
                                 Weapon(100, 1.0f, fire_bomb)};

#endif // CAVEBLITZ_SERVER_WEAPON_H
