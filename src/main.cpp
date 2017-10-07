#include <SDL2/SDL.h>

#undef main

#include "Client/Game.h"
#include "Client/MatchScene.h"
#include "Client/ServerConnection.h"

int main() {
    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::Window window("caveblitz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL2pp::Surface icon("assets/icon.png");
    window.SetIcon(icon);

    auto game = std::make_shared<Game>(Game(renderer));

    auto server_connection = std::make_shared<ServerConnection>(ServerConnection());

    auto scene = std::make_shared<MatchScene>(MatchScene(game, server_connection));

    game->set_scene(scene);
    game->run();

    return 0;
}
