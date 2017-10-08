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

class MatchScene : public Scene {
private:
    std::shared_ptr<Game> game;
    std::shared_ptr<ServerConnection> server_connection;
    std::vector<PlayerID> player_ids;
    size_t joins_sent{0};
    SDL2pp::Texture dynamic_layer;
    SDL2pp::Texture render_target;

    std::map<int32_t, bool> keys_held;

    std::map<PlayerID, std::shared_ptr<Ship>> ships;
    std::map<ProjectileID, std::shared_ptr<Projectile>> projectiles;

    void initialize_layers();

public:
    MatchScene(std::shared_ptr<Game> game, std::shared_ptr<ServerConnection> server_connection);

    bool gather_inputs();

    bool tick(DeltaTime dt);

    void join_server();

    void draw(DeltaTime dt);

    void handle_update();

    void handle_player_update(PlayerUpdate pu);

    void handle_projectile_update(ProjectileUpdate pu);

    void handle_explosion_update(ExplosionUpdate eu);

    void handle_server_join_info(ServerJoinInfo sji);
};

#endif // CAVEBLITZ_CLIENT_MATCH_SCENE_H
