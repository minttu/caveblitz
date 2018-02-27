#include "Game.h"

Game::Game(SDL2pp::Renderer &renderer)
        : scene(nullptr),
          renderer(renderer),
          null_texture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 2, 2) {
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

void Game::switch_scene(Scene *(*new_scene_fn)(Game *)) {
    auto new_scene = new_scene_fn(this);

    if (this->scene != nullptr) {
        this->scene->switched_from();
    }

    if (new_scene != nullptr) {
        new_scene->switched_to();
    }

    this->scene = new_scene;
}

SDL2pp::Point Game::window_size() const {
    return this->renderer.GetOutputSize();
}

void Game::run() {
    if (this->scene == nullptr) {
        return;
    }

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
