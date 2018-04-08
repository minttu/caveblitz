#ifndef CAVEBLITZ_SERVER_MATCH_H
#define CAVEBLITZ_SERVER_MATCH_H

#include "ServerPlayer.h"

class Match {
public:
    virtual std::vector<std::shared_ptr<ServerPlayer>> get_players() const = 0;
    virtual void set_winner(PlayerID player_id) = 0;
    virtual void set_draw() = 0;
    virtual void send_message(std::string &msg) = 0;
    virtual void respawn(PlayerID player_id) = 0;
};

#endif // CAVEBLITZ_SERVER_MATCH_H
