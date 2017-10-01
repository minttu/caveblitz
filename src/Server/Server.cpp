#include "Server.h"

const int MAX_PLAYERS = 4;

Server::Server() = default;

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
        auto span = target.subspan(offset, offset + size);

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

void Server::update(double dt) {
    for (auto const &x : this->players) {
        auto id = x.first;
        auto player = x.second;

        if (player->position.x <= 0) {
            player->collide(dt, 1, fmaxf(1, player->position.y));
        } else if (player->position.y <= 0) {
            player->collide(dt, fmaxf(1, player->position.x), 1);
        }

        try {
            auto input = this->player_inputs.at(id);

            player->update(dt,
                           static_cast<float>(input.thrust) / 128,
                           static_cast<float>(input.rotation) / 128);
        } catch (const std::out_of_range &oor) {
            continue;
        }
    }

    this->player_inputs.clear();
}

void Server::serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
    PlayerUpdate player_update{};

    for (auto const &x : this->players) {
        auto player = x.second;
        player_update.player_id = player->player_id;
        player_update.x = player->position.x;
        player_update.y = player->position.y;
        player_update.rotation = player->rotation;
        player_update.health = 100;

        player_update.serialize(target);
    }
}
