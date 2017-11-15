#include "FreeForAllGameMode.h"

FreeForAllGameMode::FreeForAllGameMode() {
}

std::string FreeForAllGameMode::get_name() {
    return "Free for all deathmatch";
}

void FreeForAllGameMode::match_start(Match & /*unused*/) {
}

bool FreeForAllGameMode::match_tick(Match &match) {
    unsigned int alive_players = 0;
    auto players = match.get_players();
    for (auto const &x : players) {
        if (x->health > 0) {
            alive_players++;
        }
    }

    return alive_players <= 1 && players.size() > alive_players;
}
