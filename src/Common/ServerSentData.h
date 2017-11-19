#ifndef CAVEBLITZ_COMMON_SERVER_SENT_DATA_H
#define CAVEBLITZ_COMMON_SERVER_SENT_DATA_H

#include "DataTransfer.h"

enum ResponseDataType : uint8_t {
    PLAYER_UPDATE = 1,
    SERVER_UPDATE = 2,
    PROJECTILE_UPDATE = 3,
    EXPLOSION_UPDATE = 4,
    SERVER_JOIN_INFO = 5,
    CLIENT_FATAL_ERROR = 6,
    PICKUP_SPAWN_UPDATE = 7,
    PICKUP_DESPAWN_UPDATE = 8,
    MATCH_RESET = 9,
    SERVER_MESSAGE = 10
};

const uint8_t RESPONSE_DATA_SIZES[] = {0, 14, 8, 12, 12, 33, 1, 6, 1, 1, 0};

enum MATCH_STATUS { MATCH_WAITING = 1, MATCH_PLAYING = 2, MATCH_ENDED = 3 };

struct ServerUpdate {
    uint32_t frame;
    uint32_t delta_ticks;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(SERVER_UPDATE));

        Bytes4 data{};
        data.i = frame;
        data.serialize(target);
        data.i = delta_ticks;
        data.serialize(target);
    }

    bool deserialize(const gsl::span<uint8_t> &target) {
        Bytes4 data{};
        data.deserialize(target, 1);
        frame = data.i;
        data.deserialize(target, 5);
        delta_ticks = data.i;

        return true;
    }
};
using ServerUpdate = struct ServerUpdate;

struct PlayerUpdate {
    PlayerID player_id;
    int8_t health;
    float x;
    float y;
    float rotation;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(PLAYER_UPDATE));
        target->push_back(player_id);
        target->push_back(static_cast<uint8_t>(health));
        Bytes4 data{};
        data.f = x;
        data.serialize(target);
        data.f = y;
        data.serialize(target);
        data.f = rotation;
        data.serialize(target);
    }

    bool deserialize(const gsl::span<uint8_t> &target) {
        if (target.size() != RESPONSE_DATA_SIZES[PLAYER_UPDATE] + 1 || target[0] != PLAYER_UPDATE) {
            return false;
        }

        player_id = target[1];
        health = target[2];
        Bytes4 data{};
        data.deserialize(target, 3);
        x = data.f;
        data.deserialize(target, 7);
        y = data.f;
        data.deserialize(target, 11);
        rotation = data.f;

        return true;
    }
};
using PlayerUpdate = struct PlayerUpdate;

struct ProjectileUpdate {
    ProjectileID projectile_id;
    PlayerID player_id;
    uint8_t projectile_type;
    float x;
    float y;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(PROJECTILE_UPDATE));
        Bytes2 data2{};
        data2.i = projectile_id;
        data2.serialize(target);
        target->push_back(player_id);
        target->push_back(projectile_type);

        Bytes4 data{};
        data.f = x;
        data.serialize(target);
        data.f = y;
        data.serialize(target);
    }

    bool deserialize(const gsl::span<uint8_t> &target) {
        Bytes2 data2{};
        data2.deserialize(target, 1);
        projectile_id = data2.i;
        player_id = target[3];
        projectile_type = target[4];

        Bytes4 data{};
        data.deserialize(target, 5);
        x = data.f;
        data.deserialize(target, 9);
        y = data.f;

        return true;
    }
};

using ProjectileUpdate = struct ProjectileUpdate;

struct ExplosionUpdate {
    ProjectileID projectile_id;
    uint8_t projectile_type;
    uint8_t explosion_size;
    float x;
    float y;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(EXPLOSION_UPDATE));
        Bytes2 data2{};
        data2.i = projectile_id;
        data2.serialize(target);
        target->push_back(projectile_type);
        target->push_back(explosion_size);

        Bytes4 data{};
        data.f = x;
        data.serialize(target);
        data.f = y;
        data.serialize(target);
    }

    bool deserialize(const gsl::span<uint8_t> &target) {
        Bytes2 data2{};
        data2.deserialize(target, 1);
        projectile_id = data2.i;
        projectile_type = target[3];
        explosion_size = target[4];

        Bytes4 data{};
        data.deserialize(target, 5);
        x = data.f;
        data.deserialize(target, 9);
        y = data.f;

        return true;
    }
};

using ExplosionUpdate = struct ExplosionUpdate;

struct ServerJoinInfo {
    PlayerID player_id;
    uint8_t map_name[32];

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(SERVER_JOIN_INFO));
        target->push_back(player_id);
        target->insert(target->end(), std::begin(map_name), std::end(map_name));
    }

    void deserialize(const gsl::span<uint8_t> &target) {
        player_id = target[1];
        memcpy(&map_name, target.data() + 2, 32);
    }
};

using ServerJoinInfo = struct ServerJoinInfo;

enum CLIENT_FATAL_ERRORS {
    CLIENT_FATAL_ERROR_NO_ERROR = 0,
    CLIENT_FATAL_ERROR_SERVER_FULL = 1,
    CLIENT_FATAL_ERROR_SERVER_STARTED = 2
};

const static std::string CLIENT_FATAL_ERROR_MESSAGE[] = {"No error",
                                                         "Match is full",
                                                         "Match has already started"};

struct ClientFatalError {
    uint8_t error_code;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(CLIENT_FATAL_ERROR));
        target->push_back(error_code);
    }

    void deserialize(const gsl::span<uint8_t> &target) {
        error_code = target[1];
    }

    void print() {
        std::cerr << "Client fatal error: " << gsl::at(CLIENT_FATAL_ERROR_MESSAGE, error_code)
                  << "\n";
    }
};

using ClientFatalError = struct ClientFatalError;

using PickupID = uint8_t;
using PickupType = uint8_t;

struct PickupSpawnUpdate {
    PickupID pickup_id;
    PickupType pickup_type;
    uint16_t x;
    uint16_t y;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(PICKUP_SPAWN_UPDATE));
        target->push_back(pickup_id);
        target->push_back(pickup_type);
        Bytes2 data{};
        data.i = x;
        data.serialize(target);
        data.i = y;
        data.serialize(target);
    }

    void deserialize(const gsl::span<uint8_t> &target) {
        pickup_id = target[1];
        pickup_type = target[2];
        Bytes2 data{};
        data.deserialize(target, 3);
        x = data.i;
        data.deserialize(target, 5);
        y = data.i;
    }
};

using PickupSpawnUpdate = struct PickupSpawnUpdate;

struct PickupDespawnUpdate {
    PickupID pickup_id;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(PICKUP_DESPAWN_UPDATE));
        target->push_back(pickup_id);
    }

    void deserialize(const gsl::span<uint8_t> &target) {
        pickup_id = target[1];
    }
};

using PickupDespawnUpdate = struct PickupDespawnUpdate;

struct MatchReset {
    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(MATCH_RESET));
        target->push_back(static_cast<uint8_t>(MATCH_RESET));
    }
};

using MatchReset = struct MatchReset;

struct ServerMessage {
    std::string message{"INVALID"};

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        if (message.length() > 254) {
            return;
        }
        target->push_back(static_cast<uint8_t>(SERVER_MESSAGE));
        target->push_back(static_cast<uint8_t>(message.length() + 1));
        for (auto const &x : message) {
            target->push_back(static_cast<uint8_t>(x));
        }
        target->push_back(0);
    }

    void deserialize(const gsl::span<uint8_t> &target) {
        uint8_t length = target[1];
        if (target.length() - 1 != length) {
            std::cerr << "Length mismatch " << target.length()
                      << " -2 != " << std::to_string(length) << "\n";
            message = "INVALID";
            return;
        }

        message = reinterpret_cast<char *>(target.data() + 2);
    }
};

using ServerMessage = struct ServerMessage;

#endif // CAVEBLITZ_COMMON_SERVER_SENT_DATA_H
