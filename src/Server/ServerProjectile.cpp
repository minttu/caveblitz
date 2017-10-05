#include "ServerProjectile.h"

ServerProjectile::ServerProjectile(PlayerID player_id,
                                   ProjectileID projectile_id,
                                   uint8_t projectile_type,
                                   glm::vec2 position,
                                   glm::vec2 velocity)
        : projectile_id(projectile_id),
          player_id(player_id),
          projectile_type(projectile_type),
          position(position),
          previous_position(position),
          velocity(velocity) {
}

void ServerProjectile::update(float dt) {
    this->previous_position = this->position;

    this->position += this->velocity * dt;

    // this->velocity *= (0.95f * 60 * dt);
}
