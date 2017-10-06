#include <memory>
#include <sstream>
#include <vector>

#include "../Common/DataTransfer.h"
#include "../Common/Image.h"
#include "../Server/Server.h"
#include "Game.h"

Game::Game()
        : sdl(SDL_INIT_VIDEO),
          window("CaveBlitz",
                 SDL_WINDOWPOS_UNDEFINED,
                 SDL_WINDOWPOS_UNDEFINED,
                 640,
                 480,
                 SDL_WINDOW_SHOWN),
          renderer(window, -1, SDL_RENDERER_ACCELERATED),
          bg(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 1024, 1024),
          _target(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())) {

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

    {
        SDL2pp::Texture::LockHandle lock = this->bg.Lock();
        auto start = static_cast<unsigned char *>(lock.GetPixels());
        int pitch = lock.GetPitch();
        auto bg_image = read_png("assets/maps/abstract/dynamic.png");
        uint32_t y = 0;
        uint32_t x = 0;
        for (const auto &i : bg_image.data) {
            start[(x * 4) + (y * pitch) + 0] = i[0];
            start[(x * 4) + (y * pitch) + 1] = i[1];
            start[(x * 4) + (y * pitch) + 2] = i[2];
            start[(x * 4) + (y * pitch) + 3] = i[3];
            x++;
            if (x == bg_image.width) {
                x = 0;
                y++;
            }
        }
    }

    this->bg.SetBlendMode(SDL_BLENDMODE_BLEND);

    SDL2pp::Texture render_target(
            this->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1024, 1024);

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

        server->update(dt);
        server->serialize(this->_target);
        this->handle_update();
        this->_target->clear();

        this->renderer.SetTarget(render_target);
        this->renderer.SetDrawColor(100, 149, 237, 255);
        this->renderer.Clear();

        this->renderer.Copy(bg, SDL2pp::Rect(0, 0, 1024, 1024), SDL2pp::Rect(0, 0, 1024, 1024));

        for (const auto &x : this->projectiles) {
            auto projectile = x.second;
            projectile->draw(&this->renderer);
        }

        ship->draw(&this->renderer, dt);

        this->renderer.SetTarget();

        auto right_x = std::round(std::max(std::min(ship->x - 320, 1024.0f - 640.0f), 0.0f));
        auto top_y = std::round(std::max(std::min(ship->y - 240, 1024.0f - 480.0f), 0.0f));

        SDL2pp::Rect view(right_x, top_y, 640, 480);

        this->renderer.Clear();
        this->renderer.Copy(render_target, view, SDL2pp::NullOpt);
        this->renderer.Present();

        dt = this->fps_manager.delay();
    }
}

void Game::handle_update() {
    gsl::span<uint8_t> target(this->_target->data(), this->_target->size());
    std::ptrdiff_t offset = 0;

    PlayerUpdate player_update{};
    ProjectileUpdate projectile_update{};
    ExplosionUpdate explosion_update{};

    while (offset < target.size()) {
        auto type = target[offset];
        if (type >= sizeof(RESPONSE_DATA_SIZES) / sizeof(RESPONSE_DATA_SIZES[0])) {
            break;
        }

        auto size = gsl::at(RESPONSE_DATA_SIZES, type) + 1;
        if (size == 1) {
            break;
        }

        auto span = target.subspan(offset, size);

        switch (type) {
        case PLAYER_UPDATE:
            if (!player_update.deserialize(span)) {
                break;
            }

            this->handle_player_update(player_update);
            break;
        case PROJECTILE_UPDATE:
            projectile_update.deserialize(span);
            this->handle_projectile_update(projectile_update);
            break;
        case EXPLOSION_UPDATE:
            explosion_update.deserialize(span);
            this->handle_explosion_update(explosion_update);
            break;
        default:
            break;
        }

        offset += size;
    }
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

void Game::handle_projectile_update(ProjectileUpdate pu) {
    if (this->projectiles.find(pu.projectile_id) == this->projectiles.end()) {
        auto created_projectile = std::make_shared<Projectile>(Projectile());
        created_projectile->texture = this->load_texture("assets/ship.png");
        this->projectiles[pu.projectile_id] = created_projectile;
    }

    auto projectile = this->projectiles[pu.projectile_id];
    projectile->x = pu.x;
    projectile->y = pu.y;
}

void Game::handle_explosion_update(ExplosionUpdate eu) {
    if (this->projectiles.find(eu.projectile_id) != this->projectiles.end()) {
        this->projectiles.erase(eu.projectile_id);
    }

    if (eu.explosion_size == 0) {
        return;
    }

    auto diameter = eu.explosion_size / 2;
    auto diameter_squared = diameter * diameter;

    int min_x = 9999;
    int min_y = 9999;
    int max_x = 0;
    int max_y = 0;
    int off_x = 9999;
    int off_y = 9999;

    std::vector<bool> offs(eu.explosion_size * eu.explosion_size);
    for (int y = 0; y < eu.explosion_size; y++) {
        for (int x = 0; x < eu.explosion_size; x++) {
            size_t offs_offset =
                    static_cast<size_t>(x) + (static_cast<size_t>(y) * eu.explosion_size);
            if (((x - diameter) * (x - diameter) + (y - diameter) * (y - diameter)) <=
                diameter_squared) {
                auto xx = x - diameter + eu.x;
                auto yy = y - diameter + eu.y;
                auto ok = (xx >= 0 && yy >= 0 && xx <= 1023 && yy <= 1023);
                offs[offs_offset] = ok;
                if (ok) {
                    if (xx < min_x) {
                        min_x = xx;
                        off_x = x;
                    }
                    if (yy < min_y) {
                        min_y = yy;
                        off_y = y;
                    }
                    if (xx > max_x) {
                        max_x = xx;
                    }
                    if (yy > max_y) {
                        max_y = yy;
                    }
                }
            } else {
                offs[offs_offset] = false;
            }
        }
    }

    if (min_x == 9999 && min_y == 9999 && max_x == 0 && max_y == 0) {
        return;
    }

    SDL2pp::Rect area(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1);

    SDL2pp::Texture::LockHandle lock = this->bg.Lock(area);
    auto start = static_cast<uint8_t *>(lock.GetPixels());
    int pitch = lock.GetPitch();

    for (int y = 0; y < area.h; y++) {
        for (int x = 0; x < area.w; x++) {
            if (offs[static_cast<size_t>(x + off_x) +
                     static_cast<size_t>(y + off_y) * eu.explosion_size]) {
                start[static_cast<int>(x * 4 + (y * pitch)) + 3] = 0;
            }
        }
    }
}
