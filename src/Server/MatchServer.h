#ifndef CAVEBLITZ_SERVER_MATCH_SERVER_H
#define CAVEBLITZ_SERVER_MATCH_SERVER_H

#include <ctime>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <vector>

#include <SDL2/SDL.h>
#include <gsl/gsl>

#include "../Common/ClientSentData.h"
#include "../Common/Image.h"
#include "../Common/Map.h"
#include "../Common/ServerSentData.h"
#include "GameMode.h"
#include "JoinError.h"
#include "Match.h"
#include "ServerPickup.h"
#include "ServerPlayer.h"
#include "ServerProjectile.h"
#include "Weapon.h"

class MatchServer : public Match {
private:
    ProjectileID next_projectile_id = 0;
    PlayerID next_player_id = 0;
    PickupID next_pickup_id = 0;
    std::shared_ptr<Map> map;
    Image dynamic_image;
    std::shared_ptr<GameMode> game_mode;

    std::map<PlayerID, std::shared_ptr<ServerPlayer>> players;
    std::map<ProjectileID, std::shared_ptr<ServerProjectile>> projectiles;
    std::map<PickupID, std::shared_ptr<ServerPickup>> pickups;
    std::vector<std::shared_ptr<ExplosionUpdate>> explosions;
    std::vector<PickupID> spawned_pickups;
    std::vector<PickupID> despawned_pickups;
    std::map<PlayerID, PlayerInput> player_inputs;
    std::vector<std::string> messages;

    std::mt19937 rng;

    bool handle_player_input(PlayerInput input);

    void check_player_collisions(std::shared_ptr<ServerPlayer> &player, float dt);

    bool check_projectile_collisions(std::shared_ptr<ServerProjectile> &prj);

    void explode_projectile(std::shared_ptr<ServerProjectile> &prj, float x, float y);

    void fire_primary(std::shared_ptr<ServerPlayer> &player);

    void fire_secondary(std::shared_ptr<ServerPlayer> &player);

    void apply_explosion(std::shared_ptr<ExplosionUpdate> &explosion);

    void check_start();

    void spawn_pickup();

    void despawn_pickup(PickupID pickup_id);

    void create_projectiles(std::vector<std::shared_ptr<ServerProjectile>> projectiles);

    void player_death_explosion(std::shared_ptr<ServerPlayer> &player);

    void motd(const std::shared_ptr<std::vector<uint8_t>> &output);

    void frag_message(PlayerID fragger, PlayerID fragged);

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
    uint8_t match_status;

    MatchServer();
    ~MatchServer() = default;

    std::shared_ptr<ServerJoinInfo> join_server();

    std::vector<PlayerID> handle_input(const std::shared_ptr<std::vector<uint8_t>> &input,
                                       const std::shared_ptr<std::vector<uint8_t>> &output);

    void update(float dt);
    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const;
    void serialize_reliable(const std::shared_ptr<std::vector<uint8_t>> &target) const;

    void join_player_id(PlayerID player_id, const std::shared_ptr<std::vector<uint8_t>> &output);

    std::vector<std::shared_ptr<ServerPlayer>> get_players() const override;
    void set_winner(PlayerID player_id) override;
    void set_draw() override;
};

#endif // CAVEBLITZ_SERVER_MATCH_SERVER_H
