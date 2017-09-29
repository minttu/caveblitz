#include "Server.h"

const int MAX_PLAYERS = 4;

Server::Server() {

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

bool Server::handle_input(std::shared_ptr<std::vector<uint8_t>> &data) {
    if (data->empty()) {
        return true;
    }

    bool good;
    gsl::span<uint8_t> span;
    PlayerInput input{};

    uint32_t offset = 0;
    while (data->size() > offset) {
        uint8_t type = (*data)[offset];
        if (type >= sizeof(INPUT_DATA_SIZES) / sizeof(INPUT_DATA_SIZES[0])) {
            SDL_Log("Bad input type");
            return false;
        }

        switch (type) {
            case PLAYER_INPUT:
                span = gsl::make_span(data->data() + offset, INPUT_DATA_SIZES[type] + 1);
                good = input.deserialize(span);
                if (good) {
                    this->handle_player_input(input);
                } else {
                    return false;
                }
                break;
            default:
                SDL_Log("Default %d", type);
                return false;
        }

        offset += INPUT_DATA_SIZES[type] + 1;
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

            player->update(dt, static_cast<float>(input.thrust) / 128, static_cast<float>(input.rotation) / 128);
        } catch (const std::out_of_range &oor) {
            continue;
        }
    }

    this->player_inputs.clear();
}

void Server::serialize(std::shared_ptr<std::vector<uint8_t>> &target) {
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
