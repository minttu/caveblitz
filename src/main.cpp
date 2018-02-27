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

#include "Server/Server.h"

#include "main.h"
#include "Client/MenuScene.h"

void server_runner(const bool *should_run, const uint16_t *port) {
    if (*should_run == false) {
        return;
    }

    Server server(*port);
    server.run(should_run);
}

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

    std::thread server_thread(server_runner, &run_server, &connect_port);

    SDL2pp::SDL sdl(SDL_INIT_VIDEO);
    SDL2pp::SDLTTF ttf;
    SDL2pp::Window window("caveblitz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL2pp::Surface icon("assets/icon.png");
    window.SetIcon(icon);

    Game game(renderer);
    game.connect_host = connect_host;
    game.connect_port = connect_port;

    game.switch_scene(MatchScene::ref);
    game.run();

    run_server = false;

    server_thread.join();

    return 0;
}
