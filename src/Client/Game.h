#ifndef CAVEBLITZ_GAME_H
#define CAVEBLITZ_GAME_H

#include <memory>
#include <iostream>
#include <map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <SDL2pp/SDL.hh>
#include <SDL2pp/Window.hh>
#include <SDL2pp/Renderer.hh>
#include <SDL2pp/Texture.hh>

#include "FPSManager.h"
#include "Ship.h"

class Game {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;
    FPSManager fps_manager;
    std::map<const char *, std::shared_ptr<SDL2pp::Texture>> textures;
    bool running;

public:
    Game();

    bool initialize();

    std::shared_ptr<SDL2pp::Texture> load_texture(const char *path);

    void run();

    ~Game();
};


#endif //CAVEBLITZ_GAME_H
