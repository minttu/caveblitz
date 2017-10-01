#ifndef CAVEBLITZ_SERVER_H
#define CAVEBLITZ_SERVER_H

#include <SDL2/SDL.h>
#include <gsl/gsl>
#include <map>
#include <memory>
#include <vector>

#include "../Common/DataTransfer.h"
#include "ServerPlayer.h"

class Server {
private:
    std::map<PlayerID, std::shared_ptr<ServerPlayer>> players;
    std::map<PlayerID, PlayerInput> player_inputs;

    bool handle_player_input(PlayerInput input);

public:
    Server();

    PlayerID join_server();

    bool handle_input(std::shared_ptr<std::vector<uint8_t>> &data);

    void update(double dt);
    void serialize(std::shared_ptr<std::vector<uint8_t>> &target) const;
};

#endif // CAVEBLITZ_SERVER_H
