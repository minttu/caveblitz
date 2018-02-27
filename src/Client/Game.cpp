#include "Game.h"

Game::Game(SDL2pp::Renderer &renderer) : renderer(renderer) {
    this->debug_font =
            std::make_shared<SDL2pp::Font>(SDL2pp::Font("assets/fonts/WorkSans-Regular.ttf", 12));
    this->menu_font =
            std::make_shared<SDL2pp::Font>(SDL2pp::Font("assets/fonts/WorkSans-Regular.ttf", 24));
}

std::shared_ptr<SDL2pp::Texture> Game::load_texture(const char *path) {
    if (this->textures.count(path) > 0) {
        return this->textures[path];
    }

    auto texture = std::make_shared<SDL2pp::Texture>(SDL2pp::Texture(this->renderer, path));

    this->textures[path] = texture;

    return texture;
}

void Game::set_scene(std::shared_ptr<Scene> scene) {
    this->scene = std::move(scene);
}

SDL2pp::Point Game::window_size() const {
    return this->renderer.GetOutputSize();
}

void Game::run() {
    DeltaTime dt = 0.0f;

    while (true) {
        if (!this->scene->tick(dt)) {
            break;
        }

        dt = this->fps_manager.delay();
    }
}

float Game::fps() const {
    return this->fps_manager.fps();
}
