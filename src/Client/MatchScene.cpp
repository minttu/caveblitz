#include "MatchScene.h"
#include "MenuScene.h"

MatchScene::MatchScene(Game *game)
        : Scene(game),
          map_width(2),
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
    this->server_connection = nullptr;
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

bool MatchScene::tick(DeltaTime dt, std::vector<Input> inputs) {
    if (this->server_connection == nullptr) {
        return true;
    }

    if (this->should_quit || this->server_connection->disconnected) {
        return false;
    }

    if (!this->gather_inputs(inputs)) {
        return false;
    }

    this->server_connection->tick();

    while (this->server_connection->connected && this->joins_sent < inputs.size()) {
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

bool MatchScene::gather_inputs(std::vector<Input> inputs) {
    if (this->player_ids.empty()) {
        return true;
    }

    this->server_connection->input_data->clear();

    unsigned input_num = 0;
    for (const auto &x : this->ships) {
        auto ship = x.second;

        if (input_num >= inputs.size()) {
            break;
        }

        Input input = inputs.at(input_num);
        PlayerInput playerInput{};

        playerInput.player_id = ship->player_id;
        playerInput.rotation = input.rotation;
        playerInput.thrust = input.thrust;
        playerInput.flags = 0;

        if (input.primary) {
            playerInput.flags |= PLAYER_INPUT_PRIMARY_USE;
        }

        if (input.special) {
            playerInput.flags |= PLAYER_INPUT_SECONDARY_USE;
            ship->ready_to_play = true;
        }

        playerInput.serialize(this->server_connection->input_data);

        input_num++;
    }

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

    for (const auto &x : this->explosions) {
        x->draw(&this->game->renderer, dt);
    }

    for (auto it = this->explosions.begin(); it != this->explosions.end();) {
        auto explosion = it->get();
        explosion->draw(&this->game->renderer, dt);
        if (explosion->destroy) {
            it = this->explosions.erase(it);
        } else {
            ++it;
        }
    }

    this->game->renderer.SetTarget();
    this->game->renderer.SetDrawColor(52, 52, 52, 255);
    this->game->renderer.Clear();

    auto size = this->game->window_size();

    auto scale_x = 2.0f;
    auto scale_y = 2.0f;
    auto viewport_height_splits = this->ships.size() > 2 ? 2 : 1;
    auto viewport_width_splits = this->ships.size() > 1 ? 2 : 1;
    auto viewport_width = std::min(675, size.GetX() / viewport_width_splits);
    auto viewport_height = std::min(540, size.GetY() / viewport_height_splits);
    auto viewport_x = 0;
    auto viewport_y = 0;
    auto x_offset = (size.GetX() - (viewport_width_splits * viewport_width)) / 2;
    auto y_offset = (size.GetY() - (viewport_height_splits * viewport_height)) / 2;

    if (this->player_ids.empty() || this->ships.empty()) {
        SDL2pp::Rect view(0, 0, size.GetX(), size.GetY());
        this->game->renderer.Copy(this->render_target, view, SDL2pp::NullOpt);
    } else {
        for (const auto &x : this->ships) {
            auto ship = x.second;

            SDL2pp::Rect view(std::round(std::max(std::min(std::round(ship->x) -
                                                                   viewport_width / (2 * scale_x),
                                                           static_cast<float>(map_width) -
                                                                   viewport_width / scale_x),
                                                  0.0f)),
                              std::round(std::max(std::min(std::round(ship->y) -
                                                                   viewport_height / (2 * scale_y),
                                                           static_cast<float>(map_height) -
                                                                   viewport_height / scale_y),
                                                  0.0f)),
                              (viewport_width - 2) / scale_x,
                              (viewport_height - 2) / scale_y);

            SDL2pp::Rect viewport(x_offset + viewport_x + 1,
                                  y_offset + viewport_y + 1,
                                  viewport_width - 2,
                                  viewport_height - 2);
            this->game->renderer.Copy(this->render_target, view, viewport);

            if (!ship->ready_to_play) {
                this->draw_ready_to_play(viewport);
            }

            if (viewport_x == 0) {
                viewport_x += viewport_width;
            } else if (viewport_y == 0) {
                viewport_y += viewport_height;
                viewport_x = 0;
            }
        }
    }

    this->draw_messages(dt);

    this->draw_debug();

    this->game->renderer.Present();
}

void MatchScene::draw_ready_to_play(SDL2pp::Rect viewport) const {
    auto press_down_to_start_texture =
            SDL2pp::Texture(game->renderer,
                            game->menu_font->RenderText_Blended("press down to start",
                                                                SDL2pp::Color{255, 255, 255, 255}));
    game->renderer.Copy(
            press_down_to_start_texture,
            SDL2pp::NullOpt,
            SDL2pp::Rect(viewport.GetX() +
                                 ((viewport.GetW() - press_down_to_start_texture.GetWidth()) / 2),
                         viewport.GetY() +
                                 (viewport.GetH() - press_down_to_start_texture.GetHeight()),
                         press_down_to_start_texture.GetWidth(),
                         press_down_to_start_texture.GetHeight()));
}

void MatchScene::draw_debug() {
    auto fps_str = "" + std::to_string((int)round(this->game->fps())) + " fps";
    auto fps_texture = SDL2pp::Texture(
            this->game->renderer,
            this->game->debug_font->RenderText_Solid(fps_str, SDL2pp::Color{255, 255, 255, 255}));
    this->game->renderer.Copy(fps_texture,
                              SDL2pp::NullOpt,
                              SDL2pp::Rect(4, 0, fps_texture.GetWidth(), fps_texture.GetHeight()));

    auto upf_str = "" + std::to_string(this->updates_in_frame) + " upf";
    auto upf_texture = SDL2pp::Texture(
            this->game->renderer,
            this->game->debug_font->RenderText_Solid(upf_str, SDL2pp::Color{255, 255, 255, 255}));
    this->updates_in_frame = 0;
    this->game->renderer.Copy(upf_texture,
                              SDL2pp::NullOpt,
                              SDL2pp::Rect(54, 0, upf_texture.GetWidth(), upf_texture.GetHeight()));
}

void MatchScene::draw_messages(DeltaTime dt) {
    if (this->show_messages_timer <= 0.0f) {
        return;
    }

    this->show_messages_timer -= dt;

    uint8_t alpha = static_cast<uint8_t>(this->show_messages_timer < 1.0f
                                                 ? std::max(0.0f, this->show_messages_timer * 255)
                                                 : 255);

    int max_messages = 8;
    auto pos = game->window_size().GetY() - 24;
    for (auto it = this->messages.rbegin(); it != this->messages.rend(); ++it) {
        int x = 4;
        size_t offset = 0;
        size_t size = it->size();
        auto chars = new char[it->size() + 1];
        strcpy(chars, it->c_str());
        SDL2pp::Color color{255, 255, 255, 255};
        SDL2pp::Color next_color{255, 255, 255, 255};

        while (offset < it->size()) {
            for (size_t i = offset; i < it->size() - 1; i++, size = i - offset) {
                if (chars[i] == '^') {
                    if (chars[i + 1] == 'r') {
                        next_color = SDL2pp::Color{255, 255, 255, 255};
                    } else if (chars[i + 1] >= '0' && chars[i + 1] <= '7') {
                        next_color = SHIP_COLORS[chars[i + 1] - '0'];
                    }

                    chars[i] = 0;
                    chars[i + 1] = 0;
                    size = i - offset;
                    break;
                }
            }
            if (strlen(chars + offset) != 0) {
                auto message_texture =
                        SDL2pp::Texture(this->game->renderer,
                                        this->game->chat_font->RenderText_Blended(chars + offset,
                                                                                  color));
                message_texture.SetAlphaMod(alpha);
                this->game->renderer.Copy(message_texture,
                                          SDL2pp::NullOpt,
                                          SDL2pp::Rect(x,
                                                       pos,
                                                       message_texture.GetWidth(),
                                                       message_texture.GetHeight()));
                x += message_texture.GetWidth();
            }
            offset += size + 2;
            color = next_color;
        }

        delete[] chars;

        pos -= 24;
        max_messages--;
        if (max_messages == 0) {
            break;
        }
    }
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
    ServerMessage server_message{};

    while (offset < target.size()) {
        auto type = target[offset];
        if (type >= sizeof(RESPONSE_DATA_SIZES) / sizeof(RESPONSE_DATA_SIZES[0])) {
            break;
        }

        auto size = gsl::at(RESPONSE_DATA_SIZES, type) + 1;
        if (type == SERVER_MESSAGE && offset + 1 < target.size()) {
            size = target[offset + 1] + 1;
        } else if (size == 1) {
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
        case MATCH_RESET:
            this->reset = true;
            break;
        case SERVER_MESSAGE:
            server_message.deserialize(span);
            this->messages.push_back(server_message.message);
            this->show_messages_timer = MESSAGE_SHOW_TIME;
            std::cerr << "SERVER MESSAGE: " << server_message.message << "\n";
            break;
        default:
            break;
        }

        this->updates_in_frame += 1;
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
    if (ship->health > pu.health) {
        ship->damage_taken();
    }
    ship->health = pu.health;
}

void MatchScene::handle_projectile_update(ProjectileUpdate pu) {
    if (this->projectiles.find(pu.projectile_id) == this->projectiles.end()) {
        auto created_projectile = std::make_shared<Projectile>(Projectile(pu.projectile_type));
        created_projectile->texture = this->game->load_texture("assets/projectiles.png");
        this->projectiles[pu.projectile_id] = created_projectile;
    }

    auto projectile = this->projectiles[pu.projectile_id];
    projectile->x = pu.x;
    projectile->y = pu.y;
    projectile->rotation = pu.rotation;
}

void MatchScene::handle_explosion_update(ExplosionUpdate eu) {
    if (this->projectiles.find(eu.projectile_id) != this->projectiles.end()) {
        this->projectiles.erase(eu.projectile_id);
    }

    if (eu.explosion_size == 0) {
        return;
    }

    auto explosion = std::make_shared<Explosion>(eu.x, eu.y, eu.explosion_size);
    explosion->texture = this->game->load_texture("assets/explosions.png");
    this->explosions.push_back(explosion);

    auto diameter = eu.explosion_size / 2;
    auto diameter_squared = diameter * diameter;

    int min_x = INT32_MAX;
    int min_y = INT32_MAX;
    int max_x = 0;
    int max_y = 0;
    int off_x = INT32_MAX;
    int off_y = INT32_MAX;

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
                        min_x = static_cast<int>(xx);
                        off_x = x;
                    }
                    if (yy < min_y) {
                        min_y = static_cast<int>(yy);
                        off_y = y;
                    }
                    if (xx > max_x) {
                        max_x = static_cast<int>(xx);
                    }
                    if (yy > max_y) {
                        max_y = static_cast<int>(yy);
                    }
                }
            } else {
                offs[offs_offset] = false;
            }
        }
    }

    if (min_x == INT32_MAX && min_y == INT32_MAX && max_x == 0 && max_y == 0) {
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
                start[x * 4 + (y * pitch) + 3] = 0;
            }
        }
    }
}

void MatchScene::handle_server_join_info(ServerJoinInfo sji) {
    this->player_ids.push_back(sji.player_id);

    if (!this->reset) {
        return;
    }
    this->state = MATCH_SCENE_LOADING;
    this->reset = false;
    this->messages.emplace_back("---");

    this->ships.clear();
    this->pickups.clear();
    this->projectiles.clear();

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

std::string MatchScene::name() const {
    return "game";
}

void MatchScene::switched_to() {
    this->server_connection = std::make_shared<ServerConnection>(
            ServerConnection(this->game->connect_host, this->game->connect_port));
}

void MatchScene::switched_from() {
    this->server_connection->disconnect();
    this->joins_sent = 0;
}
