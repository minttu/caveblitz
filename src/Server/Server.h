#ifndef CAVEBLITZ_SERVER_H
#define CAVEBLITZ_SERVER_H

#include <SDL2/SDL.h>
#include <gsl/gsl>
#include <map>
#include <memory>
#include <vector>

#include "../Common/DataTransfer.h"
#include "../Common/Image.h"
#include "ServerPlayer.h"

class Server {
private:
    std::map<PlayerID, std::shared_ptr<ServerPlayer>> players;
    std::map<PlayerID, PlayerInput> player_inputs;

    bool handle_player_input(PlayerInput input);

    void check_player_collisions(std::shared_ptr<ServerPlayer> &player, double dt);

    Image bg;

public:
    Server();

    PlayerID join_server();

    bool handle_input(const std::shared_ptr<std::vector<uint8_t>> &data);

    void update(double dt);
    void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const;
};

#endif // CAVEBLITZ_SERVER_H
