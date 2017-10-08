#include "Server.h"

const int MAX_PLAYERS = 4;

Server::Server() {
    this->bg = read_png("assets/maps/abstract/dynamic.png");
}

std::shared_ptr<ServerJoinInfo> Server::join_server() {
    if (this->players.size() >= MAX_PLAYERS) {
        throw std::runtime_error("maximum players reached");
    }

    auto player_id = this->next_player_id++;
    auto player = std::make_shared<ServerPlayer>(ServerPlayer(player_id));
    this->players[player_id] = player;

    auto join_info = std::make_shared<ServerJoinInfo>(ServerJoinInfo{});
    join_info->player_id = player_id;
    memset(&join_info->map_name, 0, 32);
    memcpy(&join_info->map_name, "abstract", 32);

    return join_info;
}

bool Server::handle_input(const std::shared_ptr<std::vector<uint8_t>> &input,
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
            } catch (const std::runtime_error &err) {
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

bool Server::handle_player_input(PlayerInput input) {
    try {
        this->players.at(input.player_id);
    } catch (const std::out_of_range &oor) {
        return false;
    }

    this->player_inputs[input.player_id] = input;

    return true;
}

void Server::check_player_collisions(std::shared_ptr<ServerPlayer> &player, float dt) {
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

    auto pos = this->bg.at(px, py);
    if (pos[3] > 0) {
        player->collide(dt, px, py);
    }
}

bool Server::check_projectile_collisions(std::shared_ptr<ServerProjectile> &prj) {
    auto max_x = this->max_x<float>();
    auto max_y = this->max_y<float>();

    auto px = this->clamp_x<>(prj->position.x);
    auto py = this->clamp_y<>(prj->position.y);

    return prj->position.x <= 0 || prj->position.x >= max_x || prj->position.y <= 0 ||
           prj->position.y >= max_y || this->bg.at(px, py)[3] > 0;
}

void Server::explode_projectile(std::shared_ptr<ServerProjectile> &prj) {
    auto explosion = std::make_shared<ExplosionUpdate>(ExplosionUpdate{});
    explosion->projectile_id = prj->projectile_id;
    explosion->projectile_type = prj->projectile_type;
    explosion->explosion_size = 32;
    explosion->x = this->clamp_x<>(prj->position.x);
    explosion->y = this->clamp_y<>(prj->position.y);
    this->explosions.push_back(explosion);

    this->apply_explosion(explosion);
}

void Server::apply_explosion(std::shared_ptr<ExplosionUpdate> &explosion) {
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

                this->bg.inv(xx, yy);
            }
        }
    }
}

void Server::fire_projectile(std::shared_ptr<ServerPlayer> &player) {
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
                             player->front_position(),
                             glm::rotate(glm::vec2(0.0f, -220.0f), rotation_rad)));

    this->projectiles[projectile_id] = projectile;
}

void Server::update(float dt) {
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

    this->player_inputs.clear();
    this->explosions.clear();

    for (auto it = this->projectiles.begin(); it != this->projectiles.end();) {
        auto projectile = it->second;

        projectile->update(dt);
        if (this->check_projectile_collisions(projectile)) {
            this->explode_projectile(projectile);
            it = this->projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void Server::serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
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

    for (auto const &explosion : this->explosions) {
        explosion->serialize(target);
    }
}
