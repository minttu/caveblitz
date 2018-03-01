#ifndef CAVEBLITZ_GAME_H
#define CAVEBLITZ_GAME_H

#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <SDL2pp/Font.hh>
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/SDL.hh>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>
#include <SDL2pp/Window.hh>

#include "../Server/Server.h"
#include "FPSManager.h"
#include "InputManager.h"
#include "Scene.h"

void server_runner(const bool *should_run, const uint16_t *port);

class Game {
private:
    FPSManager fps_manager;
    std::unordered_map<const char *, std::shared_ptr<SDL2pp::Texture>> textures;
    Scene *scene;
    std::thread *server_thread;

public:
    SDL2pp::Renderer &renderer;
    SDL2pp::Texture null_texture;
    std::shared_ptr<SDL2pp::Font> debug_font;
    std::shared_ptr<SDL2pp::Font> menu_font;

    std::string connect_host;
    uint16_t connect_port;
    bool should_server_run;

    InputManager input;

    explicit Game(SDL2pp::Renderer &renderer);
    ~Game() = default;

    // Game(const Game& other) = delete;

    std::shared_ptr<SDL2pp::Texture> load_texture(const char *path);

    void switch_scene(Scene *(*new_scene_fn)(Game *));

    SDL2pp::Point window_size() const;

    void run();

    float fps() const;

    void start_server();
    void stop_server();
};

#endif // CAVEBLITZ_GAME_H
