#ifndef CAVEBLITZ_SERVER_H
#define CAVEBLITZ_SERVER_H

#include <map>
#include <vector>
#include <memory>
#include <gsl/gsl>
#include <SDL2/sdl.h>

#include "ServerPlayer.h"

union FloatBytes {
    float f;
    uint8_t b[4];
};

enum PlayerInputFlags {
    PLAYER_INPUT_PRIMARY_USE = 0x01,
    PLAYER_INPUT_SECONDARY_USE = 0x02
};

enum InputDataType : uint8_t {
    PLAYER_INPUT = 1
};

const uint8_t INPUT_DATA_SIZES[255] = {
        0, 4
};

typedef struct PlayerInput {
    PlayerID player_id;
    std::int8_t thrust;
    std::int8_t rotation;
    std::uint8_t flags;

    void serialize(std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back((uint8_t) PLAYER_INPUT);
        target->push_back(player_id);
        target->push_back((uint8_t) thrust);
        target->push_back((uint8_t) rotation);
        target->push_back(flags);
    }

    bool deserialize(gsl::span<uint8_t> &target) {
        if (target.size() != INPUT_DATA_SIZES[PLAYER_INPUT] + 1 || target[0] != PLAYER_INPUT) {
            return false;
        }

        player_id = target[1];
        thrust = target[2];
        rotation = target[3];
        flags = target[4];

        return true;
    }

} PlayerInput;

enum ResponseDataType : uint8_t {
    PLAYER_UPDATE = 2
};

const uint8_t RESPONSE_DATA_SIZES[255] = {
        0, 0, 14
};

typedef struct PlayerUpdate {
    PlayerID player_id;
    uint8_t health;
    float x;
    float y;
    float rotation;

    void serialize(std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back((uint8_t) PLAYER_UPDATE);
        target->push_back(player_id);
        target->push_back(health);
        FloatBytes data{};
        data.f = x;
        target->push_back(data.b[0]);
        target->push_back(data.b[1]);
        target->push_back(data.b[2]);
        target->push_back(data.b[3]);
        data.f = y;
        target->push_back(data.b[0]);
        target->push_back(data.b[1]);
        target->push_back(data.b[2]);
        target->push_back(data.b[3]);
        data.f = rotation;
        target->push_back(data.b[0]);
        target->push_back(data.b[1]);
        target->push_back(data.b[2]);
        target->push_back(data.b[3]);
    }

    bool deserialize(gsl::span<uint8_t> &target) {
        if (target.size() != RESPONSE_DATA_SIZES[PLAYER_UPDATE] + 1 || target[0] != PLAYER_UPDATE) {
            return false;
        }

        player_id = target[1];
        health = target[2];
        FloatBytes data{};
        memcpy(data.b, target.data() + 3, 4);
        x = data.f;
        memcpy(data.b, target.data() + 7, 4);
        y = data.f;
        memcpy(data.b, target.data() + 11, 4);
        rotation = data.f;

        return true;
    }
} PlayerUpdate;

typedef struct ProjectileUpdate {
    PlayerID player_id;
    int projectile_id;
    int projectile_type_id;
    bool created;
    bool destroyed;
    float x;
    float y;
    float rotation;
} ProjectileUpdate;

typedef struct MapDestructionUpdate {
    int r;
    float x;
    float y;
} MapDestructionUpdate;

typedef struct GameUpdate {
    int player_update_size;
    PlayerUpdate *player_updates;
    int projectile_update_size;
    ProjectileUpdate *projectile_updates;
    int map_destruction_update_size;
    MapDestructionUpdate *map_destruction_updates;
} GameUpdate;

class Server {
private:
    std::map<PlayerID, ServerPlayer *> players;
    std::map<PlayerID, PlayerInput> player_inputs;
public:
    Server();

    PlayerID join_server();

    bool handle_input(std::shared_ptr<std::vector<uint8_t>> &data);

    bool handle_player_input(PlayerInput input);

    std::shared_ptr<std::vector<uint8_t>> update(double dt);
};


#endif //CAVEBLITZ_SERVER_H
