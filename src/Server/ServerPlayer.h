#ifndef CAVEBLITZ_SERVER_PLAYER_H
#define CAVEBLITZ_SERVER_PLAYER_H

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

    explicit ServerPlayer(PlayerID id);

    void update(double dt, float thrust, float rotation);

    void collide(double dt, float x, float y);
};

#endif // CAVEBLITZ_SERVER_PLAYER_H
