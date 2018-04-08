#ifndef CAVEBLITZ_SERVER_GAME_MODE_H
#define CAVEBLITZ_SERVER_GAME_MODE_H

#include "Match.h"
#include <string>

class GameMode {
public:
    virtual std::string get_name() = 0;
    virtual void match_start(Match &match) = 0;
    virtual void match_tick(Match &match, float dt) = 0;
    virtual void player_fragged(PlayerID fragger, PlayerID fragged) = 0;
};

#endif // CAVEBLITZ_SERVER_GAME_MODE_H
