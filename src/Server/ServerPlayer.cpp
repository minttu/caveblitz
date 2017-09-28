#include "ServerPlayer.h"

ServerPlayer::ServerPlayer(PlayerID id) {
    this->player_id = id;
    this->position = glm::vec2(100.0f, 100.0f);
    this->previous_position = this->position;
    this->speed = glm::vec2(6.0f, 6.0f);
    this->velocity = glm::vec2(0.0f, 0.0f);
    this->rotation = 0.0f;
    this->colliding = false;
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

void ServerPlayer::collide(double, float, float) {
    this->velocity = glm::vec2(0.0f, 0.0f);
    this->position = this->previous_position;
    this->colliding = true;
}
