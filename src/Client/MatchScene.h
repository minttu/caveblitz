#ifndef CAVEBLITZ_CLIENT_MATCH_SCENE_H
#define CAVEBLITZ_CLIENT_MATCH_SCENE_H

#include "FPSManager.h"
#include "Game.h"
#include "Projectile.h"
#include "Scene.h"
#include "Ship.h"

#include "../Common/DataTransfer.h"
#include "../Common/Image.h"
#include "../Server/Server.h"

class MatchScene : public Scene {
private:
    std::shared_ptr<Game> game;
    std::shared_ptr<Server> server;
    std::shared_ptr<std::vector<PlayerID>> player_ids;
    SDL2pp::Texture dynamic_layer;
    SDL2pp::Texture render_target;

    std::shared_ptr<std::vector<uint8_t>> update_data;
    std::shared_ptr<std::vector<uint8_t>> input_data;
    std::map<int32_t, bool> keys_held;

    std::map<PlayerID, std::shared_ptr<Ship>> ships;
    std::map<ProjectileID, std::shared_ptr<Projectile>> projectiles;

    void initialize_layers();

public:
    MatchScene(std::shared_ptr<Game> game,
               std::shared_ptr<Server> server,
               std::shared_ptr<std::vector<PlayerID>> player_ids);

    bool gather_inputs();

    bool tick(DeltaTime dt);

    void draw(DeltaTime dt);

    void handle_update();

    void handle_player_update(PlayerUpdate pu);

    void handle_projectile_update(ProjectileUpdate pu);

    void handle_explosion_update(ExplosionUpdate eu);
};

#endif // CAVEBLITZ_CLIENT_MATCH_SCENE_H
