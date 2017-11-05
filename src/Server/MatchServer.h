#ifndef CAVEBLITZ_SERVER_MATCH_SERVER_H
#define CAVEBLITZ_SERVER_MATCH_SERVER_H

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <SDL2/SDL.h>
#include <gsl/gsl>

#include "../Common/DataTransfer.h"
#include "../Common/Image.h"
#include "../Common/Map.h"
#include "ServerPlayer.h"
#include "ServerProjectile.h"
#include "ServerPickup.h"

class JoinError : public std::runtime_error {
public:
    JoinError(uint8_t error_code);
    uint8_t error_code;

    ClientFatalError to_client_fatal_error() const;
};

class MatchServer {
private:
    ProjectileID next_projectile_id = 0;
    PlayerID next_player_id = 0;
    std::shared_ptr<Map> map;
    Image dynamic_image;
    uint8_t match_status;

    std::map<PlayerID, std::shared_ptr<ServerPlayer>> players;
    std::map<ProjectileID, std::shared_ptr<ServerProjectile>> projectiles;
    std::map<PickupID, std::shared_ptr<ServerPickup>> pickups;
    std::vector<std::shared_ptr<ExplosionUpdate>> explosions;
    std::map<PlayerID, PlayerInput> player_inputs;

    bool handle_player_input(PlayerInput input);

    void check_player_collisions(std::shared_ptr<ServerPlayer> &player, float dt);

    bool check_projectile_collisions(std::shared_ptr<ServerProjectile> &prj);

    void explode_projectile(std::shared_ptr<ServerProjectile> &prj, float x, float y);

    void fire_projectile(std::shared_ptr<ServerPlayer> &player);

    void apply_explosion(std::shared_ptr<ExplosionUpdate> &explosion);

    void check_start();

    template <typename T>
    T max_x() {
        return static_cast<T>(this->dynamic_image.width - 1);
    }

    template <typename T>
    T max_y() {
        return static_cast<T>(this->dynamic_image.height - 1);
    }

    template <typename T = uint32_t>
    T clamp_x(float x) {
        return static_cast<T>(fminf(this->max_x<float>(), fmaxf(0.0f, roundf(x))));
    }

    template <typename T = uint32_t>
    T clamp_y(float y) {
        return static_cast<T>(fminf(this->max_y<float>(), fmaxf(0.0f, roundf(y))));
    }

public:
    MatchServer();
    ~MatchServer() = default;

    std::shared_ptr<ServerJoinInfo> join_server();

    bool handle_input(const std::shared_ptr<std::vector<uint8_t>> &input,
                      const std::shared_ptr<std::vector<uint8_t>> &output);

    void update(float dt);
    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const;
    void serialize_reliable(const std::shared_ptr<std::vector<uint8_t>> &target) const;
};

#endif // CAVEBLITZ_SERVER_MATCH_SERVER_H
