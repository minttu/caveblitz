#ifndef CAVEBLITZ_SERVER_SERVER_PLAYER_H
#define CAVEBLITZ_SERVER_SERVER_PLAYER_H

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec2.hpp>

#include "../Common/DataTransfer.h"

class ServerPlayer {
public:
    PlayerID player_id;
    glm::vec2 position;
    glm::vec2 previous_position;
    glm::vec2 velocity;
    glm::vec2 speed;
    float rotation;
    bool colliding;
    float primary_ready;

    explicit ServerPlayer(PlayerID id);

    void update(float dt, float thrust, float rotation);

    void collide(float dt, float x, float y);

    glm::vec2 front_position() const;
};

#endif // CAVEBLITZ_SERVER_SERVER_PLAYER_H
