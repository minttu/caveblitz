#ifndef CAVEBLITZ_SERVER_WEAPON_H
#define CAVEBLITZ_SERVER_WEAPON_H

#include <SDL2pp/Rect.hh>

#include "../Common/ProjectileType.h"
#include "ServerPlayer.h"
#include "ServerProjectile.h"

static std::vector<std::shared_ptr<ServerProjectile>>
fire_primary_weapon(uint8_t projectile_type,
                    float velocity,
                    std::shared_ptr<ServerPlayer> &player) {
    auto rotation_rad = player->rotation * glm::pi<float>() / 180;
    auto projectile = std::make_shared<ServerProjectile>(
            ServerProjectile(player->player_id,
                             0,
                             projectile_type,
                             player->position,
                             player->velocity +
                                     glm::rotate(glm::vec2(0.0f, -velocity), rotation_rad)));

    std::vector<std::shared_ptr<ServerProjectile>> ret;
    ret.push_back(projectile);
    return ret;
}

struct Weapon {
    float cooldown;
    std::vector<std::shared_ptr<ServerProjectile>> (*fire)(std::shared_ptr<ServerPlayer> plr);

    Weapon(float cooldown,
           std::vector<std::shared_ptr<ServerProjectile>> (*fire)(
                   std::shared_ptr<ServerPlayer> plr))
            : cooldown(cooldown), fire(fire) {
    }
};
using Weapon = struct Weapon;

const static Weapon WEAPONS[] = {Weapon(0.20f,
                                        [](std::shared_ptr<ServerPlayer> plr) {
                                            return fire_primary_weapon(0, 300.0f, plr);
                                        }),
                                 Weapon(0.25f,
                                        [](std::shared_ptr<ServerPlayer> plr) {
                                            return fire_primary_weapon(1, 220.0f, plr);
                                        }),
                                 Weapon(0.3f,
                                        [](std::shared_ptr<ServerPlayer> plr) {
                                            return fire_primary_weapon(2, 200.0f, plr);
                                        }),
                                 Weapon(0.4f,
                                        [](std::shared_ptr<ServerPlayer> plr) {
                                            return fire_primary_weapon(3, 180.0f, plr);
                                        }),
                                 Weapon(0.5f, [](std::shared_ptr<ServerPlayer> plr) {
                                     return fire_primary_weapon(4, 160.0f, plr);
                                 })};

#endif // CAVEBLITZ_SERVER_WEAPON_H
