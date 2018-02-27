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

#include "FPSManager.h"
#include "Scene.h"

class Game {
private:
    FPSManager fps_manager;
    std::unordered_map<const char *, std::shared_ptr<SDL2pp::Texture>> textures;
    Scene *scene;

public:
    SDL2pp::Renderer &renderer;
    SDL2pp::Texture null_texture;
    std::shared_ptr<SDL2pp::Font> debug_font;
    std::shared_ptr<SDL2pp::Font> menu_font;

    std::string connect_host;
    uint16_t connect_port;

    explicit Game(SDL2pp::Renderer &renderer);
    ~Game() = default;

    // Game(const Game& other) = delete;

    std::shared_ptr<SDL2pp::Texture> load_texture(const char *path);

    void switch_scene(Scene *(*new_scene_fn)(Game *));

    SDL2pp::Point window_size() const;

    void run();

    float fps() const;
};

#endif // CAVEBLITZ_GAME_H
