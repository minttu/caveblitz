#include "MatchServer.h"

const int MAX_PLAYERS = 8;

JoinError::JoinError(uint8_t error_code) : std::runtime_error(""), error_code(error_code) {
}

ClientFatalError JoinError::to_client_fatal_error() const {
    ClientFatalError err{};
    err.error_code = this->error_code;
    return err;
}

MatchServer::MatchServer() : map(std::make_shared<Map>(Map("abstract"))) {
    this->dynamic_image = this->map->get_dynamic()->image();
    this->match_status = MATCH_WAITING;
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

bool MatchServer::handle_input(const std::shared_ptr<std::vector<uint8_t>> &input,
                               const std::shared_ptr<std::vector<uint8_t>> &output) {
    if (input->empty()) {
        return true;
    }

    PlayerInput player_input{};
    std::ptrdiff_t offset = 0;
    gsl::span<uint8_t> target(input->data(), input->size());

    while (offset < target.size()) {
        uint8_t type = target[offset];
        if (type >= sizeof(INPUT_DATA_SIZES) / sizeof(INPUT_DATA_SIZES[0])) {
            return false;
        }

        auto size = gsl::at(INPUT_DATA_SIZES, type) + 1;
        auto span = target.subspan(offset, size);

        switch (type) {
        case PLAYER_INPUT:
            if (!player_input.deserialize(span)) {
                return false;
            }
            this->handle_player_input(player_input);

            break;
        case PLAYER_JOIN_SERVER:
            try {
                auto join_info = this->join_server();
                join_info->serialize(output);
            } catch (const JoinError &err) {
                auto fatal = err.to_client_fatal_error();
                fatal.print();
                fatal.serialize(output);
                break;
            }
            break;
        default:
            return false;
        }

        offset += size;
    }

    return true;
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

        if (!collision) {
            continue;
        }

        this->explode_projectile(prj, x, y);
        return true;
    }

    return false;
}

void MatchServer::explode_projectile(std::shared_ptr<ServerProjectile> &prj, float x, float y) {
    auto explosion = std::make_shared<ExplosionUpdate>(ExplosionUpdate{});
    explosion->projectile_id = prj->projectile_id;
    explosion->projectile_type = prj->projectile_type;
    explosion->explosion_size = 4;
    explosion->x = this->clamp_x<>(x);
    explosion->y = this->clamp_y<>(y);
    this->explosions.push_back(explosion);

    this->apply_explosion(explosion);
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

void MatchServer::fire_projectile(std::shared_ptr<ServerPlayer> &player) {
    if (player->primary_ready < 0) {
        return;
    }

    player->primary_ready = -0.25f;
    auto projectile_id = this->next_projectile_id++;
    auto rotation_rad = player->rotation * glm::pi<float>() / 180;
    auto projectile = std::make_shared<ServerProjectile>(
            ServerProjectile(player->player_id,
                             projectile_id,
                             1,
                             player->position,
                             player->velocity +
                                     glm::rotate(glm::vec2(0.0f, -220.0f), rotation_rad)));

    this->projectiles[projectile_id] = projectile;
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

    if (ready_to_start) {
        this->match_status = MATCH_PLAYING;
    }
}

void MatchServer::update(float dt) {
    if (this->match_status != MATCH_PLAYING) {
        this->check_start();
        return;
    }

    for (auto const &x : this->players) {
        auto id = x.first;
        auto player = x.second;
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
            this->fire_projectile(player);
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
        player_update.health = 100;

        player_update.serialize(target);
    }

    for (auto const &x : this->projectiles) {
        auto projectile = x.second;
        projectile_update.projectile_id = projectile->projectile_id;
        projectile_update.player_id = projectile->player_id;
        projectile_update.projectile_type = projectile->projectile_type;
        projectile_update.x = projectile->position.x;
        projectile_update.y = projectile->position.y;

        projectile_update.serialize(target);
    }
}

void MatchServer::serialize_reliable(const std::shared_ptr<std::vector<uint8_t>> &target) const {
    for (auto const &explosion : this->explosions) {
        explosion->serialize(target);
    }
}
