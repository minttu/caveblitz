#ifndef CAVEBLITZ_SERVER_SERVER_PLAYER_H
#define CAVEBLITZ_SERVER_SERVER_PLAYER_H

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec2.hpp>

#include "../Common/DataTransfer.h"

const static uint8_t NO_WEAPON = 250;

class ServerPlayer {
public:
    PlayerID player_id;
    glm::vec2 position;
    glm::vec2 previous_position;
    glm::vec2 velocity;
    glm::vec2 speed;
    float rotation;
    bool colliding;
    float primary_ready{0};
    bool ready_to_play{false};
    int8_t health{100};
    uint8_t primary_weapon{1};
    float secondary_ready{0};
    uint8_t secondary_weapon{NO_WEAPON};
    uint16_t secondary_ammo{0};

    explicit ServerPlayer(PlayerID id);

    void update(float dt, float thrust, float rotation);

    void collide(float dt, float x, float y);

    void take_damage(uint8_t damage);

    glm::vec2 front_position() const;
};

#endif // CAVEBLITZ_SERVER_SERVER_PLAYER_H
