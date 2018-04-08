#ifndef CAVEBLITZ_SERVER_FREE_FOR_ALL_GAME_MODE_H
#define CAVEBLITZ_SERVER_FREE_FOR_ALL_GAME_MODE_H

#include <unordered_map>

#include "GameMode.h"

class FreeForAllGameMode : public GameMode {
private:
    float time_remaining = 60.0f;
    std::unordered_map<PlayerID, int> frags;
    std::unordered_map<PlayerID, float> respawn_timer;
    int remaining[3] = {false, false, false};

public:
    FreeForAllGameMode();

    std::string get_name() override;

    void match_start(Match &match) override;

    void match_tick(Match &match, float dt) override;

    void player_fragged(PlayerID fragger, PlayerID fragged) override;
};

#endif // CAVEBLITZ_SERVER_FREE_FOR_ALL_GAME_MODE_H
