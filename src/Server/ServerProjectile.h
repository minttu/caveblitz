#ifndef CAVEBLITZ_SERVER_SERVER_PROJECTILE_H
#define CAVEBLITZ_SERVER_SERVER_PROJECTILE_H

#include <glm/vec2.hpp>

#include "../Common/DataTransfer.h"
#include "../Common/ProjectileType.h"
#include "ServerPlayer.h"

class ServerProjectile {
public:
    ProjectileID projectile_id;
    PlayerID player_id;
    uint8_t projectile_type;

    glm::vec2 position;
    glm::vec2 previous_position;
    glm::vec2 velocity;

    ProjectileType projectile_type_struct;

    bool hit{false};

    std::vector<std::shared_ptr<ServerProjectile>> (*on_hit)(std::shared_ptr<ServerProjectile> &prj,
                                                             std::shared_ptr<ServerPlayer> &plr);

    ServerProjectile(PlayerID player_id,
                     ProjectileID projectile_id,
                     uint8_t projectile_type,
                     glm::vec2 position,
                     glm::vec2 velocity);

    void update(float dt);
    uint8_t get_damage();
    uint8_t get_explosion_size();
};

#endif // CAVEBLITZ_SERVER_SERVER_PROJECTILE_H
