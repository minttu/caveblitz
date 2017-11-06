#include "MatchScene.h"

MatchScene::MatchScene(std::shared_ptr<Game> game,
                       std::shared_ptr<ServerConnection> server_connection)
        : map_width(2),
          map_height(2),
          should_quit(false),
          dynamic_layer(game->renderer,
                        SDL_PIXELFORMAT_ABGR8888,
                        SDL_TEXTUREACCESS_STREAMING,
                        map_width,
                        map_height),
          background_layer(game->renderer,
                           SDL_PIXELFORMAT_ABGR8888,
                           SDL_TEXTUREACCESS_STREAMING,
                           map_width,
                           map_height),
          render_target(game->renderer,
                        SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET,
                        map_width,
                        map_height) {

    this->server_connection = std::move(server_connection);
    this->game = std::move(game);
}

void MatchScene::load_map(const std::string &name) {
    this->map = std::make_shared<Map>(Map(name));

    this->map_width = this->map->get_width();
    this->map_height = this->map->get_height();

    this->dynamic_layer = SDL2pp::Texture(this->game->renderer,
                                          SDL_PIXELFORMAT_ABGR8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          this->map_width,
                                          this->map_height);
    this->background_layer = SDL2pp::Texture(this->game->renderer,
                                             SDL_PIXELFORMAT_ABGR8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             this->map_width,
                                             this->map_height);
    this->render_target = SDL2pp::Texture(this->game->renderer,
                                          SDL_PIXELFORMAT_ABGR8888,
                                          SDL_TEXTUREACCESS_TARGET,
                                          this->map_width,
                                          this->map_height);

    this->load_dynamic_layer();
    this->load_background_layer();
}

void MatchScene::load_dynamic_layer() {
    SDL2pp::Texture::LockHandle lock = this->dynamic_layer.Lock();
    auto start = static_cast<unsigned char *>(lock.GetPixels());
    int pitch = lock.GetPitch();
    auto dynamic_image = this->map->get_dynamic()->image();
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

void MatchScene::load_background_layer() {
    SDL2pp::Texture::LockHandle lock = this->background_layer.Lock();
    auto start = static_cast<unsigned char *>(lock.GetPixels());
    int pitch = lock.GetPitch();
    auto image = this->map->get_background()->image();
    uint32_t y = 0;
    uint32_t x = 0;
    for (const auto &i : image.data) {
        start[(x * 4) + (y * pitch) + 0] = i[0];
        start[(x * 4) + (y * pitch) + 1] = i[1];
        start[(x * 4) + (y * pitch) + 2] = i[2];
        start[(x * 4) + (y * pitch) + 3] = i[3];
        x++;
        if (x == image.width) {
            x = 0;
            y++;
        }
    }

    this->background_layer.SetBlendMode(SDL_BLENDMODE_BLEND);
}

bool MatchScene::tick(DeltaTime dt) {
    if (this->should_quit || this->server_connection->disconnected) {
        return false;
    }

    if (!this->gather_inputs()) {
        return false;
    }

    this->server_connection->tick();

    if (this->server_connection->connected && this->joins_sent == 0) {
        this->join_server();
    }

    this->handle_update();

    this->draw(dt);

    return true;
}

void MatchScene::join_server() {
    this->server_connection->join_server();
    this->joins_sent++;
}

bool MatchScene::gather_inputs() {
    SDL_Event e{};

    this->server_connection->input_data->clear();

    if (this->player_ids.empty()) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                return false;
            }
        }

        return true;
    }

    PlayerInput input{};
    input.player_id = this->player_ids[0];
    input.rotation = 0;
    input.thrust = 0;
    input.flags = 0;

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

    input.serialize(this->server_connection->input_data);

    return true;
}

void MatchScene::draw(DeltaTime dt) {
    if (this->state == MATCH_SCENE_LOADING) {
        return;
    }

    this->game->renderer.SetTarget(this->render_target);
    this->game->renderer.SetDrawColor(0, 0, 0, 255);
    this->game->renderer.Clear();

    this->game->renderer.Copy(this->background_layer,
                              SDL2pp::Rect(0, 0, map_width, map_height),
                              SDL2pp::Rect(0, 0, map_width, map_height));

    this->game->renderer.Copy(this->dynamic_layer,
                              SDL2pp::Rect(0, 0, map_width, map_height),
                              SDL2pp::Rect(0, 0, map_width, map_height));

    for (const auto &x : this->pickups) {
        auto pickup = x.second;
        pickup->draw(&this->game->renderer);
    }

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

    auto size = this->game->window_size();

    if (!this->player_ids.empty() && !this->ships.empty() &&
        this->ships.find(this->player_ids[0]) != this->ships.end()) {

        auto ship = this->ships[this->player_ids[0]];
        SDL2pp::Rect view(std::round(std::max(std::min(ship->x - size.GetX() / 2,
                                                       static_cast<float>(map_width) - size.GetX()),
                                              0.0f)),
                          std::round(
                                  std::max(std::min(ship->y - size.GetY() / 2,
                                                    static_cast<float>(map_height) - size.GetY()),
                                           0.0f)),
                          size.GetX(),
                          size.GetY());
        this->game->renderer.Copy(this->render_target, view, SDL2pp::NullOpt);
    } else {
        SDL2pp::Rect view(0, 0, size.GetX(), size.GetY());
        this->game->renderer.Copy(this->render_target, view, SDL2pp::NullOpt);
    }

    this->draw_debug();

    this->game->renderer.Present();
}

void MatchScene::draw_debug() {
    auto fps_str = "" + std::to_string((int)round(this->game->fps())) + " fps";
    auto fps_texture =
            SDL2pp::Texture(this->game->renderer,
                            this->game->font->RenderText_Solid(fps_str,
                                                               SDL2pp::Color{255, 255, 255, 255}));
    this->game->renderer.Copy(fps_texture,
                              SDL2pp::NullOpt,
                              SDL2pp::Rect(0, 0, fps_texture.GetWidth(), fps_texture.GetHeight()));

    auto upf_str = "" + std::to_string(this->server_connection->packets_processed_in_tick) + " upf";
    auto upf_texture =
            SDL2pp::Texture(this->game->renderer,
                            this->game->font->RenderText_Solid(upf_str,
                                                               SDL2pp::Color{255, 255, 255, 255}));
    this->game->renderer.Copy(upf_texture,
                              SDL2pp::NullOpt,
                              SDL2pp::Rect(50, 0, upf_texture.GetWidth(), upf_texture.GetHeight()));
}

void MatchScene::handle_update() {
    gsl::span<uint8_t> target(this->server_connection->update_data->data(),
                              this->server_connection->update_data->size());
    std::ptrdiff_t offset = 0;

    PlayerUpdate player_update{};
    ProjectileUpdate projectile_update{};
    ExplosionUpdate explosion_update{};
    ServerJoinInfo server_join_info{};
    ClientFatalError client_fatal_error{};
    PickupSpawnUpdate pickup_spawn_update{};
    PickupDespawnUpdate pickup_despawn_update{};

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
        case SERVER_JOIN_INFO:
            server_join_info.deserialize(span);
            this->handle_server_join_info(server_join_info);
            break;
        case PICKUP_SPAWN_UPDATE:
            pickup_spawn_update.deserialize(span);
            this->handle_pickup_spawn_update(pickup_spawn_update);
            break;
        case PICKUP_DESPAWN_UPDATE:
            pickup_despawn_update.deserialize(span);
            this->handle_pickup_despawn_update(pickup_despawn_update);
            break;
        case CLIENT_FATAL_ERROR:
            client_fatal_error.deserialize(span);
            client_fatal_error.print();
            this->server_connection->disconnect();
            break;
        default:
            break;
        }

        offset += size;
    }
}

void MatchScene::handle_player_update(PlayerUpdate pu) {
    if (this->ships.find(pu.player_id) == this->ships.end()) {
        auto create_ship = std::make_shared<Ship>(Ship(pu.player_id));
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
                auto ok = (xx >= 0 && yy >= 0 && xx <= map_width - 1 && yy <= map_height - 1);
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

void MatchScene::handle_server_join_info(ServerJoinInfo sji) {
    this->player_ids.push_back(sji.player_id);

    if (this->player_ids.size() > 1) {
        return;
    }

    std::string map_name(reinterpret_cast<char *>(sji.map_name));
    this->load_map(map_name);
    this->state = MATCH_SCENE_PLAYING;
}

void MatchScene::handle_pickup_spawn_update(PickupSpawnUpdate psu) {
    if (this->pickups.find(psu.pickup_id) == this->pickups.end()) {
        auto created_pickup = std::make_shared<Pickup>(Pickup());
        created_pickup->texture = this->game->load_texture("assets/pickup.png");
        this->pickups[psu.pickup_id] = created_pickup;
    }

    auto pickup = this->pickups[psu.pickup_id];
    pickup->x = psu.x;
    pickup->y = psu.y;
}

void MatchScene::handle_pickup_despawn_update(PickupDespawnUpdate pdu) {
    if (this->pickups.find(pdu.pickup_id) != this->pickups.end()) {
        this->pickups.erase(pdu.pickup_id);
    }
}
