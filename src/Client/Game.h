#ifndef CAVEBLITZ_GAME_H
#define CAVEBLITZ_GAME_H

#include <iostream>
#include <map>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <SDL2pp/Renderer.hh>
#include <SDL2pp/SDL.hh>
#include <SDL2pp/Texture.hh>
#include <SDL2pp/Window.hh>

#include "FPSManager.h"
#include "Projectile.h"
#include "Ship.h"

#include "../Common/DataTransfer.h"

class Game {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    SDL2pp::Texture bg;
    FPSManager fps_manager;
    std::map<const char *, std::shared_ptr<SDL2pp::Texture>> textures;
    std::map<PlayerID, std::shared_ptr<Ship>> ships;
    std::map<ProjectileID, std::shared_ptr<Projectile>> projectiles;
    std::shared_ptr<std::vector<uint8_t>> _target;
    bool running{false};

public:
    Game();

    std::shared_ptr<SDL2pp::Texture> load_texture(const char *path);

    void run();

    void handle_update();

    void handle_player_update(PlayerUpdate pu);

    void handle_projectile_update(ProjectileUpdate pu);

    void handle_explosion_update(ExplosionUpdate eu);
};

#endif // CAVEBLITZ_GAME_H
