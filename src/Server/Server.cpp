#include "Server.h"

const int MAX_PLAYERS = 4;

Server::Server() : projectile_id(0) {
    this->bg = read_png("assets/maps/abstract/dynamic.png");
}

PlayerID Server::join_server() {
    if (this->players.size() >= MAX_PLAYERS) {
        return 255;
    }
    auto id = static_cast<PlayerID>(this->players.size() + 1);
    auto player = std::make_shared<ServerPlayer>(ServerPlayer(id));
    this->players[id] = player;

    return id;
}

bool Server::handle_input(const std::shared_ptr<std::vector<uint8_t>> &data) {
    if (data->empty()) {
        return true;
    }

    PlayerInput input{};
    std::ptrdiff_t offset = 0;
    gsl::span<uint8_t> target(data->data(), data->size());

    while (offset < target.size()) {
        uint8_t type = target[offset];
        if (type >= sizeof(INPUT_DATA_SIZES) / sizeof(INPUT_DATA_SIZES[0])) {
            return false;
        }

        auto size = gsl::at(INPUT_DATA_SIZES, type) + 1;
        auto span = target.subspan(offset, size);

        switch (type) {
        case PLAYER_INPUT:
            if (!input.deserialize(span)) {
                return false;
            }
            this->handle_player_input(input);

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
    if (player->position.x <= 0) {
        player->collide(dt, 1, fmaxf(1, player->position.y));
    } else if (player->position.x >= 1023) {
        player->collide(dt, 1023, fminf(1023, player->position.y));
    } else if (player->position.y <= 0) {
        player->collide(dt, fmaxf(1, player->position.x), 1);
    } else if (player->position.y >= 1023) {
        player->collide(dt, fminf(1023, player->position.x), 1023);
    }

    auto px = static_cast<uint32_t>(fminf(1023.0f, fmaxf(0.0f, roundf(player->position.x))));
    auto py = static_cast<uint32_t>(fminf(1023.0f, fmaxf(0.0f, roundf(player->position.y))));

    auto pos = this->bg.at(px, py);
    if (pos[3] > 0) {
        player->collide(dt, px, py);
    }
}

bool Server::check_projectile_collisions(std::shared_ptr<ServerProjectile> &prj) {
    auto px = static_cast<uint32_t>(fminf(1023.0f, fmaxf(0.0f, roundf(prj->position.x))));
    auto py = static_cast<uint32_t>(fminf(1023.0f, fmaxf(0.0f, roundf(prj->position.y))));

    return prj->position.x <= 0 || prj->position.x >= 1023 || prj->position.y <= 0 ||
           prj->position.y >= 1023 || this->bg.at(px, py)[3] > 0;
}

void Server::explode_projectile(std::shared_ptr<ServerProjectile> &prj) {
    auto explosion = std::make_shared<ExplosionUpdate>(ExplosionUpdate{});
    explosion->projectile_id = prj->projectile_id;
    explosion->projectile_type = prj->projectile_type;
    explosion->explosion_size = 32;
    explosion->x = fminf(1023.0f, fmaxf(0.0f, roundf(prj->position.x)));
    explosion->y = fminf(1023.0f, fmaxf(0.0f, roundf(prj->position.y)));

    this->explosions.push_back(explosion);
}

void Server::fire_projectile(std::shared_ptr<ServerPlayer> &player) {
    auto projectile_id = ++this->projectile_id;
    auto rotation_rad = player->rotation * glm::pi<float>() / 180;
    auto projectile = std::make_shared<ServerProjectile>(
            ServerProjectile(player->player_id,
                             projectile_id,
                             1,
                             player->position,
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
