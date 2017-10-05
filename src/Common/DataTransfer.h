#ifndef CAVEBLITZ_DATA_TRANSFER_H
#define CAVEBLITZ_DATA_TRANSFER_H

#include <gsl/gsl>
#include <memory>
#include <vector>

union FloatBytes {
    float f;
    uint32_t i;
    uint8_t b[4];

    inline void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(b[0]);
        target->push_back(b[1]);
        target->push_back(b[2]);
        target->push_back(b[3]);
    }

    inline void deserialize(const gsl::span<uint8_t> &target, uint32_t offset) {
        memcpy(&b, target.data() + offset, 4);
    }
};

enum PlayerInputFlags { PLAYER_INPUT_PRIMARY_USE = 0x01, PLAYER_INPUT_SECONDARY_USE = 0x02 };

enum InputDataType : uint8_t { PLAYER_INPUT = 1 };

const uint8_t INPUT_DATA_SIZES[255] = {0, 4};

using PlayerID = uint8_t;
using ProjectileID = uint8_t; // TODO: uint16_t

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

enum ResponseDataType : uint8_t {
    PLAYER_UPDATE = 1,
    SERVER_UPDATE = 2,
    PROJECTILE_UPDATE = 3,
    EXPLOSION_UPDATE = 4
};

const uint8_t RESPONSE_DATA_SIZES[255] = {0, 14, 8, 11, 11};

struct ServerUpdate {
    uint32_t frame;
    uint32_t delta_ticks;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(SERVER_UPDATE));

        FloatBytes data{};
        data.i = frame;
        data.serialize(target);
        data.i = delta_ticks;
        data.serialize(target);
    }

    bool deserialize(const gsl::span<uint8_t> &target) {
        FloatBytes data{};
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
    uint8_t health;
    float x;
    float y;
    float rotation;

    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(static_cast<uint8_t>(PLAYER_UPDATE));
        target->push_back(player_id);
        target->push_back(health);
        FloatBytes data{};
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
        FloatBytes data{};
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
        target->push_back(projectile_id);
        target->push_back(player_id);
        target->push_back(projectile_type);

        FloatBytes data{};
        data.f = x;
        data.serialize(target);
        data.f = y;
        data.serialize(target);
    }

    bool deserialize(const gsl::span<uint8_t> &target) {

        projectile_id = target[1];
        player_id = target[2];
        projectile_type = target[3];

        FloatBytes data{};
        data.deserialize(target, 4);
        x = data.f;
        data.deserialize(target, 8);
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
        target->push_back(projectile_id);
        target->push_back(projectile_type);
        target->push_back(explosion_size);

        FloatBytes data{};
        data.f = x;
        data.serialize(target);
        data.f = y;
        data.serialize(target);
    }

    bool deserialize(const gsl::span<uint8_t> &target) {

        projectile_id = target[1];
        projectile_type = target[2];
        explosion_size = target[3];

        FloatBytes data{};
        data.deserialize(target, 4);
        x = data.f;
        data.deserialize(target, 8);
        y = data.f;

        return true;
    }
};

using ExplosionUpdate = struct ExplosionUpdate;

#endif // CAVEBLITZ_DATA_TRANSFER_h
