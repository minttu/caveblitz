#include "FreeForAllGameMode.h"
#include <sstream>

FreeForAllGameMode::FreeForAllGameMode() {
}

std::string FreeForAllGameMode::get_name() {
    return "deathmatch";
}

void FreeForAllGameMode::match_start(Match & /*unused*/) {
}

void FreeForAllGameMode::match_tick(Match &match, float dt) {
    auto players = match.get_players();

    for (auto &x : this->respawn_timer) {
        x.second -= dt;
    }

    for (auto const &player : players) {
        auto respawn = this->respawn_timer.find(player->player_id);
        if (respawn != this->respawn_timer.end() && respawn->second < 0.0f) {
            this->respawn_timer.erase(player->player_id);
            match.respawn(player->player_id);
        }
    }

    if (this->time_remaining < 5.0f && !this->remaining[0]) {
        this->remaining[0] = true;
        std::string remaining = "^65^r sec remaining";
        match.send_message(remaining);
    } else if (this->time_remaining < 15.0f && !this->remaining[1]) {
        this->remaining[1] = true;
        std::string remaining = "^615^r sec remaining";
        match.send_message(remaining);
    } else if (this->time_remaining < 30.0f && !this->remaining[2]) {
        this->remaining[2] = true;
        std::string remaining = "^630^r sec remaining";
        match.send_message(remaining);
    }

    this->time_remaining -= dt;
    if (time_remaining <= 0.0f) {
        int most_frags = -1;
        PlayerID player = 0;
        bool draw = true;
        for (auto const &x : this->frags) {
            if (x.second > most_frags) {
                player = x.first;
                most_frags = x.second;
                draw = false;
            } else if (x.second == most_frags) {
                draw = true;
            }
        }

        if (draw) {
            match.set_draw();
        } else {
            match.set_winner(player);
        }
    }
}

void FreeForAllGameMode::player_fragged(PlayerID fragger, PlayerID fragged) {
    if (this->frags.find(fragger) == this->frags.end()) {
        this->frags[fragger] = 1;
    } else {
        this->frags[fragger] = this->frags[fragger] + 1;
    }

    this->respawn_timer[fragged] = 3.0f;
}
