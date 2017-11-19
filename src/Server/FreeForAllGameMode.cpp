#include "FreeForAllGameMode.h"

FreeForAllGameMode::FreeForAllGameMode() {
}

std::string FreeForAllGameMode::get_name() {
    return "Free for all deathmatch";
}

void FreeForAllGameMode::match_start(Match & /*unused*/) {
}

void FreeForAllGameMode::match_tick(Match &match) {
    unsigned int alive_players = 0;
    auto players = match.get_players();
    std::shared_ptr<ServerPlayer> alive_player = nullptr;
    for (auto const &x : players) {
        if (x->health > 0) {
            alive_players++;
            alive_player = x;
        }
    }

    if (alive_players <= 1 && players.size() > alive_players) {
        match.set_winner(alive_player->player_id);
    }
}
