#ifndef CAVEBLITZ_SERVER_FREE_FOR_ALL_GAME_MODE_H
#define CAVEBLITZ_SERVER_FREE_FOR_ALL_GAME_MODE_H

#include "GameMode.h"

class FreeForAllGameMode : public GameMode {
public:
    FreeForAllGameMode();

    std::string get_name() override;

    void match_start(Match &match) override;

    void match_tick(Match &match) override;
};

#endif // CAVEBLITZ_SERVER_FREE_FOR_ALL_GAME_MODE_H
