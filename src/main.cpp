#include <cstdlib>
#include <iostream>
#include <thread>

#include <SDL2/SDL.h>
#include <enet/enet.h>
#include <CLI/CLI.hpp>
#include <SDL2pp/SDLTTF.hh>

#undef main

#include "Client/Game.h"
#include "Client/MatchScene.h"

int main(int argc, char **argv) {
    if (enet_initialize() != 0) {
        std::cerr << "enet_initialize failed\n";
        return 1;
    }
    atexit(enet_deinitialize);

    CLI::App app("caveblitz");

    bool run_server = false;
    app.add_flag("-s,--server", run_server, "Start server");

    std::string connect_host = "localhost";
    app.add_option("-c,--connect", connect_host, "Host to connect to");

    uint16_t connect_port = 30320;
    app.add_option("-p,--port", connect_port, "Port to connect to");

    CLI11_PARSE(app, argc, argv);

    SDL2pp::SDL sdl(SDL_INIT_EVERYTHING);
    SDL2pp::SDLTTF ttf;
    SDL2pp::Window window("caveblitz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_GameControllerAddMappingsFromFile("assets/gamecontrollerdb.txt");

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL2pp::Surface icon("assets/icon.png");
    window.SetIcon(icon);

    Game game(renderer);
    game.connect_host = connect_host;
    game.connect_port = connect_port;
    game.should_server_run = run_server;

    game.start_server();

    game.switch_scene(MatchScene::ref);
    game.run();

    game.stop_server();

    return 0;
}
