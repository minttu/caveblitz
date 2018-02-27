#include "MatchServer.h"
#include "FreeForAllGameMode.h"

static const int MAX_PLAYERS = 8;
static const int MAX_PICKUPS = 64;

MatchServer::MatchServer() : map(std::make_shared<Map>(Map("abstract"))) {
    this->dynamic_image = this->map->get_dynamic()->image();
    this->match_status = MATCH_WAITING;
    this->game_mode = std::make_shared<FreeForAllGameMode>(FreeForAllGameMode());
    this->rng.seed(static_cast<uint32_t>(time(nullptr)));
}

std::shared_ptr<ServerJoinInfo> MatchServer::join_server() {
    if (this->players.size() >= MAX_PLAYERS) {
        throw JoinError(CLIENT_FATAL_ERROR_SERVER_FULL);
    }

    if (this->match_status != MATCH_WAITING) {
        throw JoinError(CLIENT_FATAL_ERROR_SERVER_STARTED);
    }

    auto player_id = this->next_player_id++;
    auto player = std::make_shared<ServerPlayer>(ServerPlayer(player_id));
    auto point = this->map->get_player_spawn(player_id);
    player->position = glm::vec2(point->x, point->y);
    this->players[player_id] = player;

    auto join_info = std::make_shared<ServerJoinInfo>(ServerJoinInfo{});
    join_info->player_id = player_id;
    memset(&join_info->map_name, 0, 32);
    strncpy(reinterpret_cast<char *>(&join_info->map_name[0]), this->map->name.c_str(), 32);

    return join_info;
}

std::vector<PlayerID>
MatchServer::handle_input(const std::shared_ptr<std::vector<uint8_t>> &input,
                          const std::shared_ptr<std::vector<uint8_t>> &output) {
    if (input->empty()) {
        return std::vector<PlayerID>();
    }

    std::vector<PlayerID> joined_players;

    PlayerInput player_input{};
    std::ptrdiff_t offset = 0;
    gsl::span<uint8_t> target(input->data(), input->size());

    while (offset < target.size()) {
        uint8_t type = target[offset];
        if (type >= sizeof(INPUT_DATA_SIZES) / sizeof(INPUT_DATA_SIZES[0])) {
            break;
        }

        auto size = gsl::at(INPUT_DATA_SIZES, type) + 1;
        auto span = target.subspan(offset, size);

        switch (type) {
        case PLAYER_INPUT:
            if (!player_input.deserialize(span)) {
                break;
            }
            this->handle_player_input(player_input);

            break;
        case PLAYER_JOIN_SERVER:
            try {
                auto join_info = this->join_server();
                join_info->serialize(output);
                joined_players.push_back(join_info->player_id);
                this->motd(output);
            } catch (const JoinError &err) {
                auto fatal = err.to_client_fatal_error();
                fatal.print();
                fatal.serialize(output);
                break;
            }
            break;
        default:
            break;
        }

        offset += size;
    }

    return joined_players;
}

void MatchServer::motd(const std::shared_ptr<std::vector<uint8_t>> &output) {
    ServerMessage server_message;
    std::stringstream ss;
    ss << "Welcome to caveblitz! ";
    ss << "Playing on map: '";
    ss << this->map->name;
    ss << "' with game mode: '";
    ss << this->game_mode->get_name();
    ss << "'";
    server_message.message = ss.str();
    server_message.serialize(output);
}

void MatchServer::frag_message(PlayerID fragger, PlayerID fragged) {
    std::stringstream ss;
    ss << "'";
    ss << std::to_string(fragger);
    ss << "' fragged '";
    ss << std::to_string(fragged);
    ss << "'";
    this->messages.push_back(ss.str());
}

void MatchServer::join_player_id(PlayerID player_id,
                                 const std::shared_ptr<std::vector<uint8_t>> &output) {
    this->next_player_id = player_id;
    try {
        auto join_info = this->join_server();
        join_info->serialize(output);
        this->motd(output);
    } catch (const JoinError &err) {
        auto fatal = err.to_client_fatal_error();
        fatal.print();
        fatal.serialize(output);
    }
}

bool MatchServer::handle_player_input(PlayerInput input) {
    try {
        this->players.at(input.player_id);
    } catch (const std::out_of_range &oor) {
        return false;
    }

    this->player_inputs[input.player_id] = input;
    if ((input.flags & PLAYER_INPUT_SECONDARY_USE) != 0) {
        this->players[input.player_id]->ready_to_play = true;
    }

    return true;
}

void MatchServer::check_player_collisions(std::shared_ptr<ServerPlayer> &player, float dt) {
    auto max_x = this->max_x<float>();
    auto max_y = this->max_y<float>();

    if (player->position.x <= 0) {
        player->collide(dt, 1, fmaxf(1, player->position.y));
    } else if (player->position.x >= max_x) {
        player->collide(dt, max_x, fminf(max_y, player->position.y));
    } else if (player->position.y <= 0) {
        player->collide(dt, fmaxf(1, player->position.x), 1);
    } else if (player->position.y >= max_y) {
        player->collide(dt, fminf(max_x, player->position.x), max_y);
    }

    auto px = this->clamp_x<>(player->position.x);
    auto py = this->clamp_y<>(player->position.y);

    auto pos = this->dynamic_image.at(px, py);
    if (pos[3] > 0) {
        player->collide(dt, px, py);
    }

    for (auto const &x : this->pickups) {
        auto pickup = x.second;
        auto dist = sqrt(pow(pickup->x - px + 8, 2) + pow(pickup->y - py + 8, 2));
        if (dist < 24) {
            auto weapon = gsl::at(WEAPONS, pickup->pickup_type);
            if (weapon.is_primary) {
                player->primary_weapon = pickup->pickup_type;
                player->primary_ready = weapon.cooldown;
            } else {
                player->secondary_weapon = pickup->pickup_type;
                player->secondary_ammo = weapon.max_ammo;
                player->secondary_ready = weapon.cooldown;
            }

            this->despawn_pickup(x.first);
            this->spawn_pickup();
            break;
        }
    }
}

bool MatchServer::check_projectile_collisions(std::shared_ptr<ServerProjectile> &prj) {
    auto max_x = this->max_x<float>();
    auto max_y = this->max_y<float>();

    auto x = this->clamp_x<>(prj->previous_position.x);
    auto y = this->clamp_y<>(prj->previous_position.y);
    auto delta = prj->position - prj->previous_position;
    auto steps = glm::length(delta) * 0.5f;
    auto per_step = delta / steps;

    for (int i = 0; i < steps; i++) {
        x += per_step.x;
        y += per_step.y;

        auto over_map = x <= 0 || x >= max_x || y <= 0 || y >= max_y;
        auto collision = over_map ||
                         this->dynamic_image.at(static_cast<uint16_t>(round(x)),
                                                static_cast<uint16_t>(round(y)))[3] > 0;

        if (collision) {
            this->explode_projectile(prj, x, y);
            return true;
        }

        for (auto const &player_pair : this->players) {
            auto player = player_pair.second;
            if (player->player_id == prj->player_id || player->health == 0) {
                continue;
            }

            auto dist = sqrt(pow(x - player->position.x, 2) + pow(y - player->position.y, 2));
            if (dist < 12) {
                this->explode_projectile(prj, x, y);
                return true;
            }
        }
    }

    return false;
}

void MatchServer::create_projectiles(std::vector<std::shared_ptr<ServerProjectile>> projectiles) {
    for (const auto &projectile : projectiles) {
        auto projectile_id = this->next_projectile_id++;
        projectile->projectile_id = projectile_id;
        this->projectiles[projectile_id] = projectile;
    }
}

void MatchServer::explode_projectile(std::shared_ptr<ServerProjectile> &prj, float x, float y) {
    if (prj->hit) {
        return;
    }
    prj->hit = true;
    if (prj->on_hit != nullptr) {
        this->create_projectiles(prj->on_hit(prj, this->players[prj->player_id]));
    }

    auto explosion = std::make_shared<ExplosionUpdate>(ExplosionUpdate{});
    explosion->projectile_id = prj->projectile_id;
    explosion->projectile_type = prj->projectile_type;
    explosion->explosion_size = prj->get_explosion_size();
    explosion->x = this->clamp_x<>(x);
    explosion->y = this->clamp_y<>(y);
    this->explosions.push_back(explosion);
    for (auto const &player_pair : this->players) {
        auto player = player_pair.second;
        if (player->player_id == prj->player_id || player->health == 0) {
            continue;
        }

        auto dist = sqrt(pow(x - player->position.x, 2) + pow(y - player->position.y, 2));
        if (dist < 12 + (explosion->explosion_size / 2)) {
            player->take_damage(prj->get_damage());
            if (player->health == 0) {
                this->frag_message(prj->player_id, player->player_id);
                this->player_death_explosion(player);
            }
        }
    }

    this->apply_explosion(explosion);
}

void MatchServer::player_death_explosion(std::shared_ptr<ServerPlayer> &player) {
    auto projectile = std::make_shared<ServerProjectile>(
            ServerProjectile(player->player_id, 254, 5, player->position, glm::vec2()));
    this->explode_projectile(projectile, player->position.x, player->position.y);
}

void MatchServer::apply_explosion(std::shared_ptr<ExplosionUpdate> &explosion) {
    auto max_x = this->max_x<int>();
    auto max_y = this->max_y<int>();

    auto diameter = explosion->explosion_size / 2;
    auto diameter_squared = diameter * diameter;
    for (int y = 0; y < explosion->explosion_size; y++) {
        for (int x = 0; x < explosion->explosion_size; x++) {
            if (((x - diameter) * (x - diameter) + (y - diameter) * (y - diameter)) <=
                diameter_squared) {
                auto xx = x - diameter + explosion->x;
                auto yy = y - diameter + explosion->y;
                if (xx < 0 || yy < 0 || xx > max_x || yy > max_y) {
                    continue;
                }

                this->dynamic_image.inv(xx, yy);
            }
        }
    }
}

void MatchServer::fire_primary(std::shared_ptr<ServerPlayer> &player) {
    if (player->primary_ready < 0) {
        return;
    }

    auto weapon = gsl::at(WEAPONS, player->primary_weapon);

    player->primary_ready = -weapon.cooldown;
    this->create_projectiles(weapon.fire(player));
}

void MatchServer::fire_secondary(std::shared_ptr<ServerPlayer> &player) {
    if (player->secondary_ready < 0 || player->secondary_ammo <= 0 ||
        player->secondary_weapon == NO_WEAPON) {
        return;
    }

    auto weapon = gsl::at(WEAPONS, player->secondary_weapon);

    player->secondary_ready = -weapon.cooldown;
    this->create_projectiles(weapon.fire(player));
}

void MatchServer::check_start() {
    if (this->players.empty() || this->match_status != MATCH_WAITING) {
        return;
    }

    auto ready_to_start = true;
    for (auto const &x : this->players) {
        auto player = x.second;
        if (!player->ready_to_play) {
            ready_to_start = false;
            break;
        }
    }

    if (!ready_to_start) {
        return;
    }

    this->match_status = MATCH_PLAYING;

    this->messages.emplace_back("Starting match");
    this->game_mode->match_start(*this);

    for (int i = 0; i < 10; i++) {
        this->spawn_pickup();
    }
}

void MatchServer::spawn_pickup() {
    if (this->pickups.size() > MAX_PICKUPS) {
        return;
    }

    PickupID id = this->next_pickup_id++;
    while (this->pickups.count(id) == 1) {
        id++;
    }

    std::uniform_int_distribution<uint16_t> x_dist(1, this->max_x<uint16_t>());
    std::uniform_int_distribution<uint16_t> y_dist(1, this->max_x<uint16_t>());
    // TODO: make powerful weapons rarer
    std::uniform_int_distribution<uint8_t> weapon_dist(0,
                                                       (sizeof(WEAPONS) / sizeof(WEAPONS[0])) - 1);

    uint16_t x = x_dist(this->rng);
    uint16_t y = y_dist(this->rng);

    while (this->dynamic_image.at(x, y)[3] != 0) {
        x = x_dist(this->rng);
        y = y_dist(this->rng);
    }

    auto pickup = std::make_shared<ServerPickup>(ServerPickup(id, weapon_dist(this->rng), x, y));
    this->pickups[id] = pickup;
    this->spawned_pickups.push_back(id);
}

void MatchServer::despawn_pickup(PickupID pickup_id) {
    this->despawned_pickups.push_back(pickup_id);
}

void MatchServer::update(float dt) {
    if (this->match_status != MATCH_PLAYING) {
        this->check_start();
        return;
    }

    this->spawned_pickups.clear();
    this->despawned_pickups.clear();
    this->messages.clear();

    for (auto const &it : this->players) {
        auto id = it.first;
        auto player = it.second;
        if (player->health == 0) {
            continue;
        }

        PlayerInput input{};

        try {
            input = this->player_inputs.at(id);
        } catch (const std::out_of_range &oor) {
            input.rotation = 0;
            input.thrust = 0;
            input.flags = 0;
        }

        player->update(dt,
                       static_cast<float>(input.thrust) / 128,
                       static_cast<float>(input.rotation) / 128);

        this->check_player_collisions(player, dt);

        if ((input.flags & PLAYER_INPUT_PRIMARY_USE) != 0) {
            this->fire_primary(player);
        }

        if ((input.flags & PLAYER_INPUT_SECONDARY_USE) != 0) {
            this->fire_secondary(player);
        }
    }

    this->explosions.clear();

    for (auto it = this->projectiles.begin(); it != this->projectiles.end();) {
        auto projectile = it->second;

        projectile->update(dt);
        if (this->check_projectile_collisions(projectile)) {
            it = this->projectiles.erase(it);
        } else {
            ++it;
        }
    }

    for (auto const &x : this->despawned_pickups) {
        this->pickups.erase(x);
    }

    this->game_mode->match_tick(*this);
}

void MatchServer::set_winner(PlayerID player_id) {
    this->set_draw();

    if (this->players.find(player_id) != this->players.end()) {
        this->players[player_id]->winner = true;
    }
}

void MatchServer::set_draw() {
    this->match_status = MATCH_ENDED;

    this->spawned_pickups.clear();
    this->despawned_pickups.clear();
    this->explosions.clear();
    this->messages.emplace_back("Match ended");
}

void MatchServer::serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
    PlayerUpdate player_update{};
    ProjectileUpdate projectile_update{};

    for (auto const &x : this->players) {
        auto player = x.second;
        player_update.player_id = player->player_id;
        player_update.x = player->position.x;
        player_update.y = player->position.y;
        player_update.rotation = player->rotation;
        player_update.health = player->health;

        player_update.serialize(target);
    }

    for (auto const &x : this->projectiles) {
        auto projectile = x.second;
        projectile_update.projectile_id = projectile->projectile_id;
        projectile_update.player_id = projectile->player_id;
        projectile_update.projectile_type = projectile->projectile_type;
        projectile_update.x = projectile->position.x;
        projectile_update.y = projectile->position.y;
        projectile_update.rotation = projectile->rotation;

        projectile_update.serialize(target);
    }
}

void MatchServer::serialize_reliable(const std::shared_ptr<std::vector<uint8_t>> &target) const {
    for (auto const &explosion : this->explosions) {
        explosion->serialize(target);
    }

    PickupSpawnUpdate pickup_spawn_update{};
    PickupDespawnUpdate pickup_despawn_update{};
    ServerMessage server_message{};

    for (auto const &id : this->spawned_pickups) {
        auto pickup = this->pickups.at(id);
        pickup_spawn_update.pickup_id = pickup->pickup_id;
        pickup_spawn_update.pickup_type = pickup->pickup_type;
        pickup_spawn_update.x = pickup->x;
        pickup_spawn_update.y = pickup->y;
        pickup_spawn_update.serialize(target);
    }

    for (auto const &id : this->despawned_pickups) {
        pickup_despawn_update.pickup_id = id;
        pickup_despawn_update.serialize(target);
    }

    for (auto const &str : this->messages) {
        server_message.message = str;
        server_message.serialize(target);
    }
}

std::vector<std::shared_ptr<ServerPlayer>> MatchServer::get_players() const {
    std::vector<std::shared_ptr<ServerPlayer>> ret;
    for (auto const &x : this->players) {
        ret.push_back(x.second);
    }
    return ret;
}
