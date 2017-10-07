#include "MatchScene.h"

MatchScene::MatchScene(std::shared_ptr<Game> game,
                       std::shared_ptr<Server> server,
                       std::shared_ptr<std::vector<PlayerID>> player_ids)
        : dynamic_layer(game->renderer,
                        SDL_PIXELFORMAT_ABGR8888,
                        SDL_TEXTUREACCESS_STREAMING,
                        1024,
                        1024),
          render_target(game->renderer,
                        SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET,
                        1024,
                        1024),
          update_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())),
          input_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())) {

    this->server = std::move(server);
    this->game = std::move(game);
    this->player_ids = std::move(player_ids);
    this->input_data->reserve(1024);
    this->update_data->reserve(1024);
    this->initialize_layers();
}

void MatchScene::initialize_layers() {
    SDL2pp::Texture::LockHandle lock = this->dynamic_layer.Lock();
    auto start = static_cast<unsigned char *>(lock.GetPixels());
    int pitch = lock.GetPitch();
    auto dynamic_image = read_png("assets/maps/abstract/dynamic.png");
    uint32_t y = 0;
    uint32_t x = 0;
    for (const auto &i : dynamic_image.data) {
        start[(x * 4) + (y * pitch) + 0] = i[0];
        start[(x * 4) + (y * pitch) + 1] = i[1];
        start[(x * 4) + (y * pitch) + 2] = i[2];
        start[(x * 4) + (y * pitch) + 3] = i[3];
        x++;
        if (x == dynamic_image.width) {
            x = 0;
            y++;
        }
    }

    this->dynamic_layer.SetBlendMode(SDL_BLENDMODE_BLEND);
}

bool MatchScene::tick(DeltaTime dt) {
    // main thread ?
    if (!this->gather_inputs()) {
        return false;
    }
    this->server->handle_input(this->input_data);
    this->input_data->clear();

    // network thread ?
    this->server->update(dt);
    this->server->serialize(this->update_data);
    this->handle_update();
    this->update_data->clear();

    this->draw(dt);

    return true;
}

bool MatchScene::gather_inputs() {
    PlayerInput input{};
    input.player_id = this->player_ids->front();
    input.rotation = 0;
    input.thrust = 0;
    input.flags = 0;

    SDL_Event e{};
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            return false;
        }
        if (e.type == SDL_KEYDOWN) {
            this->keys_held[e.key.keysym.sym] = true;
        }
        if (e.type == SDL_KEYUP) {
            this->keys_held[e.key.keysym.sym] = false;
        }
    }

    if (this->keys_held[SDLK_UP]) {
        input.thrust += 127;
    }

    if (this->keys_held[SDLK_RIGHT]) {
        input.rotation -= 127;
    }

    if (this->keys_held[SDLK_LEFT]) {
        input.rotation += 127;
    }

    if (this->keys_held[SDLK_RSHIFT]) {
        input.flags |= PLAYER_INPUT_PRIMARY_USE;
    }

    if (this->keys_held[SDLK_DOWN]) {
        input.flags |= PLAYER_INPUT_SECONDARY_USE;
    }

    // main thread ?
    input.serialize(this->input_data);

    return true;
}

void MatchScene::draw(DeltaTime dt) {
    this->game->renderer.SetTarget(this->render_target);
    this->game->renderer.SetDrawColor(100, 149, 237, 255);
    this->game->renderer.Clear();

    this->game->renderer.Copy(this->dynamic_layer,
                              SDL2pp::Rect(0, 0, 1024, 1024),
                              SDL2pp::Rect(0, 0, 1024, 1024));

    for (const auto &x : this->projectiles) {
        auto projectile = x.second;
        projectile->draw(&this->game->renderer);
    }

    for (const auto &x : this->ships) {
        auto ship = x.second;
        ship->draw(&this->game->renderer, dt);
    }

    this->game->renderer.SetTarget();
    this->game->renderer.Clear();

    auto ship = this->ships.begin()->second;
    SDL2pp::Rect view(std::round(std::max(std::min(ship->x - 320, 1024.0f - 640.0f), 0.0f)),
                      std::round(std::max(std::min(ship->y - 240, 1024.0f - 480.0f), 0.0f)),
                      640,
                      480);
    this->game->renderer.Copy(this->render_target, view, SDL2pp::NullOpt);

    this->game->renderer.Present();
}

void MatchScene::handle_update() {
    gsl::span<uint8_t> target(this->update_data->data(), this->update_data->size());
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

void MatchScene::handle_player_update(PlayerUpdate pu) {
    if (this->ships.find(pu.player_id) == this->ships.end()) {
        auto create_ship = std::make_shared<Ship>(Ship());
        create_ship->texture = this->game->load_texture("assets/ship.png");
        this->ships[pu.player_id] = create_ship;
    }

    auto ship = this->ships[pu.player_id];

    ship->x = pu.x;
    ship->y = pu.y;
    ship->rotation = pu.rotation;
}

void MatchScene::handle_projectile_update(ProjectileUpdate pu) {
    if (this->projectiles.find(pu.projectile_id) == this->projectiles.end()) {
        auto created_projectile = std::make_shared<Projectile>(Projectile());
        created_projectile->texture = this->game->load_texture("assets/ship.png");
        this->projectiles[pu.projectile_id] = created_projectile;
    }

    auto projectile = this->projectiles[pu.projectile_id];
    projectile->x = pu.x;
    projectile->y = pu.y;
}

void MatchScene::handle_explosion_update(ExplosionUpdate eu) {
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

    SDL2pp::Texture::LockHandle lock = this->dynamic_layer.Lock(area);
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