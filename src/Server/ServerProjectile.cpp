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
          velocity(velocity),
          projectile_type_struct(gsl::at(PROJECTILE_TYPES, projectile_type)) {
    this->on_hit = nullptr;
}

void ServerProjectile::update(float dt) {
    this->previous_position = this->position;

    this->velocity += glm::vec2(0, 40.0f * dt);
    this->position += this->velocity * dt;
}

uint8_t ServerProjectile::get_damage() {
    return this->projectile_type_struct.damage;
}

uint8_t ServerProjectile::get_explosion_size() {
    return this->projectile_type_struct.explosion_size;
}