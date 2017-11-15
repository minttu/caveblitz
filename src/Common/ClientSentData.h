#ifndef CAVEBLITZ_COMMON_CLIENT_SENT_DATA_H
#define CAVEBLITZ_COMMON_CLIENT_SENT_DATA_H

#include "DataTransfer.h"

enum PlayerInputFlags { PLAYER_INPUT_PRIMARY_USE = 0x01, PLAYER_INPUT_SECONDARY_USE = 0x02 };

enum InputDataType { PLAYER_INPUT = 1, PLAYER_JOIN_SERVER = 2 };

const uint8_t INPUT_DATA_SIZES[] = {0, 4, 1};

struct PlayerInput {
    PlayerID player_id;
    std::int8_t thrust;
    std::int8_t rotation;
    std::uint8_t flags;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(PLAYER_INPUT));
        target->push_back(player_id);
        target->push_back(static_cast<uint8_t>(thrust));
        target->push_back(static_cast<uint8_t>(rotation));
        target->push_back(flags);
    }

    bool deserialize(const gsl::span<uint8_t> &target) {
        if (target.size() != INPUT_DATA_SIZES[PLAYER_INPUT] + 1 || target[0] != PLAYER_INPUT) {
            return false;
        }

        player_id = target[1];
        thrust = target[2];
        rotation = target[3];
        flags = target[4];

        return true;
    }
};
using PlayerInput = struct PlayerInput;

struct PlayerJoinServer {
    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(PLAYER_JOIN_SERVER));
        target->push_back(static_cast<uint8_t>(PLAYER_JOIN_SERVER));
    }
};
using PlayerJoinServer = struct PlayerJoinServer;

#endif // CAVEBLITZ_COMMON_CLIENT_SENT_DATA_H
