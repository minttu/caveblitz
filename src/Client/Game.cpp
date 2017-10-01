#include <memory>
#include <sstream>
#include <vector>

#include "../Common/DataTransfer.h"
#include "../Server/Server.h"
#include "Game.h"

Game::Game()
        : sdl(SDL_INIT_VIDEO),
          window("CaveBlitz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN),
          renderer(window, -1, SDL_RENDERER_ACCELERATED),
          _target(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())),
          running(false) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL2pp::Surface icon("assets/icon.png");
    this->window.SetIcon(icon);
}

std::shared_ptr<SDL2pp::Texture> Game::load_texture(const char *path) {
    if (this->textures.find(path) != this->textures.end()) {
        return this->textures[path];
    }

    auto texture = std::make_shared<SDL2pp::Texture>(SDL2pp::Texture(this->renderer, path));

    this->textures[path] = texture;

    return texture;
}

void Game::run() {
    this->running = true;

    SDL_Event e{};

    auto server = std::make_unique<Server>(Server());

    PlayerID player_id = server->join_server();
    auto ship = std::make_shared<Ship>(Ship());
    this->ships[player_id] = ship;
    ship->texture = this->load_texture("assets/ship.png");

    auto input_data = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>());
    input_data->reserve(1024);

    std::map<Sint32, bool> keys_held;
    DeltaTime dt = 0.0f;

    while (this->running) {
        input_data->clear();

        PlayerInput input{};
        input.player_id = player_id;
        input.rotation = 0;
        input.thrust = 0;
        input.flags = 0;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                this->running = false;
            }
            if (e.type == SDL_KEYDOWN) {
                keys_held[e.key.keysym.sym] = true;
            }
            if (e.type == SDL_KEYUP) {
                keys_held[e.key.keysym.sym] = false;
            }
        }

        if (keys_held[SDLK_UP]) {
            input.thrust += 127;
        }

        if (keys_held[SDLK_RIGHT]) {
            input.rotation -= 127;
        }

        if (keys_held[SDLK_LEFT]) {
            input.rotation += 127;
        }

        if (keys_held[SDLK_RSHIFT]) {
            input.flags |= PLAYER_INPUT_PRIMARY_USE;
        }

        if (keys_held[SDLK_DOWN]) {
            input.flags |= PLAYER_INPUT_SECONDARY_USE;
        }

        input.serialize(input_data);

        server->handle_input(input_data);

        this->renderer.SetDrawColor(100, 149, 237);
        this->renderer.Clear();

        server->update(dt);
        server->serialize(this->_target);
        this->handle_update();

        ship->draw(&this->renderer, dt);

        this->renderer.Present();

        dt = this->fps_manager.delay();
    }
}

void Game::handle_update() {
    gsl::span<uint8_t> target(this->_target->data(), this->_target->size());
    std::ptrdiff_t offset = 0;

    PlayerUpdate player_update{};

    while (offset < target.size()) {
        auto type = target[offset];
        if (type >= sizeof(RESPONSE_DATA_SIZES) / sizeof(RESPONSE_DATA_SIZES[0])) {
            break;
        }

        auto size = gsl::at(RESPONSE_DATA_SIZES, type) + 1;
        if (size == 1) {
            break;
        }

        auto span = target.subspan(offset, offset + size);

        switch(type) {
            case PLAYER_UPDATE:
                if (!player_update.deserialize(span)) {
                    break;
                }

                this->handle_player_update(player_update);
                break;
            default:
                break;
        }

        offset += size;
    }

    this->_target->clear();
}

void Game::handle_player_update(PlayerUpdate pu) {
    if (this->ships.find(pu.player_id) == this->ships.end()) {
        return;
    }

    auto ship = this->ships[pu.player_id];

    ship->x = pu.x;
    ship->y = pu.y;
    ship->rotation = pu.rotation;
}


Game::~Game() = default;
