#ifndef CAVEBLITZ_SERVER_SERVER_H
#define CAVEBLITZ_SERVER_SERVER_H

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

#include <enet/enet.h>

#include "MatchServer.h"

struct PeerInfo {
    std::vector<PlayerID> players;
    int match_version;

    explicit PeerInfo(int match_version) {
        this->match_version = match_version;
    }
};
using PeerInfo = struct PeerInfo;

class Server {
private:
    std::shared_ptr<std::vector<uint8_t>> update_data;
    std::shared_ptr<std::vector<uint8_t>> input_data;
    ENetHost *server;
    std::shared_ptr<MatchServer> match;
    int match_version{0};

    void next_match();

public:
    explicit Server(uint16_t port);
    ~Server();

    void run(const bool *should_run);
};

#endif // CAVEBLITZ_SERVER_SERVER_H
