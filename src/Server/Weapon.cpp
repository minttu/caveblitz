#include "Weapon.h"

std::shared_ptr<ServerProjectile>
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

std::vector<std::shared_ptr<ServerProjectile>>
fire_primary_weapon(uint8_t projectile_type,
                    float velocity,
                    std::shared_ptr<ServerPlayer> &player) {
    auto projectile = create_projectile(projectile_type, velocity, player);

    std::vector<std::shared_ptr<ServerProjectile>> ret;
    ret.push_back(projectile);
    return ret;
}

std::vector<std::shared_ptr<ServerProjectile>>
morning_star_on_hit(std::shared_ptr<ServerProjectile> &prj, std::shared_ptr<ServerPlayer> &player) {
    std::vector<std::shared_ptr<ServerProjectile>> ret;
    for (int i = 1; i <= 36; i++) {
        auto rotation_rad = (i * 10) * glm::pi<float>() / 180;
        auto projectile = std::make_shared<ServerProjectile>(
                ServerProjectile(player->player_id,
                                 0,
                                 PELLET_2_PROJECTILE,
                                 prj->position,
                                 glm::rotate(glm::vec2(0.0f, -200.0f), rotation_rad)));
        ret.push_back(projectile);
    }
    return ret;
}

void morning_star_on_update(ServerProjectile &prj, float dt) {
    prj.rotation += dt * 1000;
    if (prj.rotation > 360) {
        prj.rotation -= 360;
    }
}

std::vector<std::shared_ptr<ServerProjectile>>
fire_morning_star(std::shared_ptr<ServerPlayer> &player) {
    auto projectile = create_projectile(MORNING_STAR_PROJECTILE, 200.0f, player);
    projectile->on_hit = morning_star_on_hit;
    projectile->on_update = morning_star_on_update;

    std::vector<std::shared_ptr<ServerProjectile>> ret;
    ret.push_back(projectile);
    return ret;
}

std::vector<std::shared_ptr<ServerProjectile>> fire_bomb(std::shared_ptr<ServerPlayer> &player) {
    auto projectile = create_projectile(BOMB_PROJECTILE, 0, player);
    projectile->velocity += glm::vec2(0.0f, 50.0f);

    std::vector<std::shared_ptr<ServerProjectile>> ret;
    ret.push_back(projectile);
    return ret;
}
