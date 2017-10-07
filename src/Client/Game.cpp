#include "Game.h"

Game::Game(SDL2pp::Renderer &renderer) : renderer(renderer) {
}

std::shared_ptr<SDL2pp::Texture> Game::load_texture(const char *path) {
    if (this->textures.find(path) != this->textures.end()) {
        return this->textures[path];
    }

    auto texture = std::make_shared<SDL2pp::Texture>(SDL2pp::Texture(this->renderer, path));

    this->textures[path] = texture;

    return texture;
}

void Game::set_scene(std::shared_ptr<Scene> scene) {
    this->scene = std::move(scene);
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
