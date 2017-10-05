#ifndef CAVEBLITZ_SERVER_SERVER_PROJECTILE_H
#define CAVEBLITZ_SERVER_SERVER_PROJECTILE_H

#include <glm/vec2.hpp>

#include "../Common/DataTransfer.h"

class ServerProjectile {
public:
    ProjectileID projectile_id;
    PlayerID player_id;
    uint8_t projectile_type;

    glm::vec2 position;
    glm::vec2 previous_position;
    glm::vec2 velocity;

    ServerProjectile(PlayerID player_id,
                     ProjectileID projectile_id,
                     uint8_t projectile_type,
                     glm::vec2 position,
                     glm::vec2 velocity);

    void update(float dt);
};

#endif // CAVEBLITZ_SERVER_SERVER_PROJECTILE_H
