#include "ServerPlayer.h"

ServerPlayer::ServerPlayer(PlayerID id)
        : player_id(id),
          position(glm::vec2(100.0f, 100.0f)),
          previous_position(position),
          velocity(glm::vec2(0.0f, 0.0f)),
          speed(glm::vec2(6.0f, 6.0f)),
          rotation(0.0f),
          colliding(false) {
}

void ServerPlayer::update(double dt, float thrust, float rotation) {
    this->previous_position = this->position;

    this->rotation -= rotation * 90 * dt;
    this->velocity += glm::rotate(glm::vec2(0.0f, -thrust * dt), this->rotation * glm::pi<float>() / 180);
    if (!this->colliding) {
        this->position += this->velocity * this->speed;
    }
    this->colliding = false;
    this->velocity *= (0.95f * 60 * dt);
}

void ServerPlayer::collide(double /*unused*/, float /*unused*/, float /*unused*/) {
    this->velocity = glm::vec2(0.0f, 0.0f);
    this->position = this->previous_position;
    this->colliding = true;
}
