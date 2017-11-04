#ifndef CAVEBLITZ_GAME_H
#define CAVEBLITZ_GAME_H

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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
    std::map<const char *, std::shared_ptr<SDL2pp::Texture>> textures;
    std::shared_ptr<Scene> scene;

public:
    SDL2pp::Renderer &renderer;

    explicit Game(SDL2pp::Renderer &renderer);
    ~Game() = default;

    // Game(const Game& other) = delete;

    std::shared_ptr<SDL2pp::Texture> load_texture(const char *path);

    void set_scene(std::shared_ptr<Scene> scene);

    SDL2pp::Point window_size() const;

    void run();
};

#endif // CAVEBLITZ_GAME_H
