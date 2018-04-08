#include "Game.h"

Game::Game(SDL2pp::Renderer &renderer)
        : scene(nullptr),
          renderer(renderer),
          null_texture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 2, 2) {
    this->debug_font =
            std::make_shared<SDL2pp::Font>(SDL2pp::Font("assets/fonts/WorkSans-Regular.ttf", 12));
    this->menu_font =
            std::make_shared<SDL2pp::Font>(SDL2pp::Font("assets/fonts/WorkSans-Regular.ttf", 24));
    this->chat_font =
            std::make_shared<SDL2pp::Font>(SDL2pp::Font("assets/fonts/WorkSans-Regular.ttf", 18));
    this->server_thread = nullptr;
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
    std::vector<Input> inputs;

    while (true) {
        this->input.player_inputs(&inputs);
        if (this->input.should_quit) {
            break;
        }

        if (!this->scene->tick(dt, inputs)) {
            break;
        }

        dt = this->fps_manager.delay();
    }
}

float Game::fps() const {
    return this->fps_manager.fps();
}

void server_runner(const bool *should_run, const uint16_t *port) {
    Server server(*port);
    server.run(should_run);
}

void Game::start_server() {
    if (this->server_thread != nullptr || !this->should_server_run) {
        return;
    }

    this->server_thread =
            new std::thread(server_runner, &this->should_server_run, &this->connect_port);
}

void Game::stop_server() {
    if (this->server_thread == nullptr) {
        return;
    }

    this->should_server_run = false;
    this->server_thread->join();
    delete this->server_thread;
    this->server_thread = nullptr;
}
