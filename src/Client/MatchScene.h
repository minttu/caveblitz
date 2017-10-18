#ifndef CAVEBLITZ_CLIENT_MATCH_SCENE_H
#define CAVEBLITZ_CLIENT_MATCH_SCENE_H

#include <iostream>

#include "FPSManager.h"
#include "Game.h"
#include "Projectile.h"
#include "Scene.h"
#include "ServerConnection.h"
#include "Ship.h"

#include "../Common/DataTransfer.h"
#include "../Common/Image.h"
#include "../Common/Map.h"

enum MatchSceneState { MATCH_SCENE_LOADING = 1, MATCH_SCENE_PLAYING = 2 };

class MatchScene : public Scene {
private:
    std::shared_ptr<Game> game;
    std::shared_ptr<ServerConnection> server_connection;
    std::vector<PlayerID> player_ids;
    size_t joins_sent{0};
    MatchSceneState state{MATCH_SCENE_LOADING};

    std::shared_ptr<Map> map;
    SDL2pp::Texture dynamic_layer;
    SDL2pp::Texture background_layer;
    SDL2pp::Texture render_target;

    std::map<int32_t, bool> keys_held;

    std::map<PlayerID, std::shared_ptr<Ship>> ships;
    std::map<ProjectileID, std::shared_ptr<Projectile>> projectiles;

    void load_map(const std::string &name);

    void load_dynamic_layer();

    void load_background_layer();

public:
    MatchScene(std::shared_ptr<Game> game, std::shared_ptr<ServerConnection> server_connection);

    bool gather_inputs();

    bool tick(DeltaTime dt) override;

    void join_server();

    void draw(DeltaTime dt);

    void handle_update();

    void handle_player_update(PlayerUpdate pu);

    void handle_projectile_update(ProjectileUpdate pu);

    void handle_explosion_update(ExplosionUpdate eu);

    void handle_server_join_info(ServerJoinInfo sji);
};

#endif // CAVEBLITZ_CLIENT_MATCH_SCENE_H
