#ifndef CAVEBLITZ_CLIENT_MATCH_SCENE_H
#define CAVEBLITZ_CLIENT_MATCH_SCENE_H

#include <iostream>
#include <unordered_map>

#include "Explosion.h"
#include "FPSManager.h"
#include "Pickup.h"
#include "Projectile.h"
#include "Scene.h"
#include "ServerConnection.h"
#include "Ship.h"

#include "../Common/ClientSentData.h"
#include "../Common/Image.h"
#include "../Common/Map.h"
#include "../Common/ServerSentData.h"

enum MatchSceneState { MATCH_SCENE_LOADING = 1, MATCH_SCENE_PLAYING = 2 };

class MatchScene : public Scene {
private:
    int map_width;
    int map_height;
    bool should_quit;
    int updates_in_frame{0};
    bool reset{true};

    std::shared_ptr<ServerConnection> server_connection;
    std::vector<PlayerID> player_ids;
    size_t joins_sent{0};
    MatchSceneState state{MATCH_SCENE_LOADING};

    std::shared_ptr<Map> map;
    SDL2pp::Texture dynamic_layer;
    SDL2pp::Texture background_layer;
    SDL2pp::Texture render_target;

    std::map<PlayerID, std::shared_ptr<Ship>> ships;
    std::map<ProjectileID, std::shared_ptr<Projectile>> projectiles;
    std::map<PickupID, std::shared_ptr<Pickup>> pickups;
    std::vector<std::shared_ptr<Explosion>> explosions;

    void load_map(const std::string &name);

    void load_dynamic_layer();

    void load_background_layer();

    void draw(DeltaTime dt);

    void handle_update();

    void draw_debug();

    void draw_ready_to_play() const;

    void handle_player_update(PlayerUpdate pu);

    void handle_projectile_update(ProjectileUpdate pu);

    void handle_explosion_update(ExplosionUpdate eu);

    void handle_pickup_spawn_update(PickupSpawnUpdate psu);

    void handle_pickup_despawn_update(PickupDespawnUpdate pdu);

    void handle_server_join_info(ServerJoinInfo sji);

    void join_server();

    bool gather_inputs(std::vector<Input> vector);

public:
    explicit MatchScene(Game *game);

    std::string name() const override;
    bool tick(DeltaTime dt, std::vector<Input> inputs) override;
    void switched_to() override;
    void switched_from() override;

    static Scene *ref(Game *game) {
        return new MatchScene(game);
    }
};

#endif // CAVEBLITZ_CLIENT_MATCH_SCENE_H
