#include "ServerPlayer.h"

ServerPlayer::ServerPlayer(PlayerID id)
        : player_id(id),
          position(glm::vec2(100.0f, 100.0f)),
          previous_position(position),
          velocity(glm::vec2(0.0f, 0.0f)),
          speed(glm::vec2(6.0f, 6.0f)),
          rotation(0.0f),
          colliding(false),
          primary_ready(0.0f) {
}

void ServerPlayer::update(float dt, float thrust, float rotation) {
    this->previous_position = this->position;
    this->primary_ready += dt;

    this->rotation -= rotation * 120.0f * dt;
    auto rotation_rad = this->rotation * glm::pi<float>() / 180.0f;
    this->velocity += glm::rotate(glm::vec2(0.0f, -thrust * dt * 60.0f), rotation_rad);
    this->velocity += glm::vec2(0, 3.0f * dt);
    if (!this->colliding) {
        this->position += this->velocity * this->speed * dt;
    }
    this->colliding = false;
    this->velocity -= this->velocity * (dt * 2.0f);
}

void ServerPlayer::collide(float /*unused*/, float /*unused*/, float /*unused*/) {
    this->velocity = glm::vec2(0.0f, 0.0f);
    this->position = this->previous_position;
    this->colliding = true;
}

glm::vec2 ServerPlayer::front_position() const {
    auto rotation_rad = this->rotation * glm::pi<float>() / 180;
    return this->position + glm::rotate(glm::vec2(0, -16.0f), rotation_rad);
}
