#include <memory>
#include <sstream>
#include <vector>
#include "Game.h"
#include "../Server/Server.h"

Game::Game()
        : sdl(SDL_INIT_VIDEO),
          window("CaveBlitz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN),
          renderer(window, -1, SDL_RENDERER_ACCELERATED),
          running(false) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL2pp::Surface icon("assets/icon.png");
    this->window.SetIcon(icon);
}

bool Game::initialize() {

    return true;
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

    auto server = new Server;

    PlayerID player_id = server->join_server();
    auto ship = new Ship;
    ship->texture = this->load_texture("assets/ship.png");

    std::map<Sint32, bool> keys_held;

    DeltaTime dt = 0.0f;

    while (this->running) {
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

        if (keys_held[SDLK_UP])
            input.thrust += 127;

        if (keys_held[SDLK_RIGHT])
            input.rotation -= 127;

        if (keys_held[SDLK_LEFT])
            input.rotation += 127;

        if (keys_held[SDLK_RSHIFT])
            input.flags |= PLAYER_INPUT_PRIMARY_USE;

        if (keys_held[SDLK_DOWN])
            input.flags |= PLAYER_INPUT_SECONDARY_USE;

        auto input_data = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>());
        input.serialize(input_data);

        server->handle_input(input_data);

        this->renderer.SetDrawColor(100, 149, 237);
        this->renderer.Clear();

        auto response = server->update(dt);

        bool good;
        gsl::span<uint8_t> span;
        PlayerUpdate update{};

        uint32_t offset = 0;
        while (response->size() > offset) {
            uint8_t type = (*response)[offset];
            switch (type) {
                case PLAYER_UPDATE:
                    span = gsl::make_span(response->data() + offset, RESPONSE_DATA_SIZES[type] + 1);
                    good = update.deserialize(span);
                    if (good) {
                        ship->x = update.x;
                        ship->y = update.y;
                        ship->rotation = update.rotation;
                    }
                    break;
                default:
                    good = false;
                    break;
            }
            if (!good) {
                break;
            }

            offset += RESPONSE_DATA_SIZES[type] + 1;
        }

        ship->draw(&this->renderer, dt);

        this->renderer.Present();

        dt = this->fps_manager.delay();
    }
}

Game::~Game() = default;
