#ifndef CAVEBLITZ_SERVER_WEAPON_H
#define CAVEBLITZ_SERVER_WEAPON_H

#include <SDL2pp/Rect.hh>

#include "../Common/ProjectileType.h"
#include "ServerPlayer.h"
#include "ServerProjectile.h"

static std::shared_ptr<ServerProjectile>
create_projectile(uint8_t projectile_type, float velocity, std::shared_ptr<ServerPlayer> &player) {
    auto rotation_rad = player->rotation * glm::pi<float>() / 180;
    auto projectile = std::make_shared<ServerProjectile>(
            ServerProjectile(player->player_id,
                             0,
                             projectile_type,
                             player->position,
                             player->velocity +
                                     glm::rotate(glm::vec2(0.0f, -velocity), rotation_rad)));

    return projectile;
}

static std::vector<std::shared_ptr<ServerProjectile>>
fire_primary_weapon(uint8_t projectile_type,
                    float velocity,
                    std::shared_ptr<ServerPlayer> &player) {
    auto projectile = create_projectile(projectile_type, velocity, player);

    std::vector<std::shared_ptr<ServerProjectile>> ret;
    ret.push_back(projectile);
    return ret;
}

static std::vector<std::shared_ptr<ServerProjectile>>
morning_star_on_hit(std::shared_ptr<ServerProjectile> &prj, std::shared_ptr<ServerPlayer> &player) {
    std::vector<std::shared_ptr<ServerProjectile>> ret;
    for (int i = 1; i <= 36; i++) {
        auto rotation_rad = (i * 10) * glm::pi<float>() / 180;
        auto projectile = std::make_shared<ServerProjectile>(
                ServerProjectile(player->player_id,
                                 0,
                                 1,
                                 prj->position,
                                 glm::rotate(glm::vec2(0.0f, -200.0f), rotation_rad)));
        ret.push_back(projectile);
    }
    return ret;
}

static std::vector<std::shared_ptr<ServerProjectile>>
fire_morning_star(std::shared_ptr<ServerPlayer> &player) {
    auto projectile = create_projectile(4, 200.0f, player);
    projectile->on_hit = morning_star_on_hit;

    std::vector<std::shared_ptr<ServerProjectile>> ret;
    ret.push_back(projectile);
    return ret;
}

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
                                 Weapon(100, 1.0f, fire_morning_star)};

#endif // CAVEBLITZ_SERVER_WEAPON_H
