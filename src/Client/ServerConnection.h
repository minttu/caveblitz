#ifndef CAVEBLITZ_CLIENT_SERVER_CONNECTION_H
#define CAVEBLITZ_CLIENT_SERVER_CONNECTION_H

#include <iostream>
#include <memory>
#include <vector>

#include <enet/enet.h>

#include "../Common/DataTransfer.h"

class ServerConnection {
private:
    ENetHost *client;
    ENetPeer *peer;

public:
    std::shared_ptr<std::vector<uint8_t>> update_data;
    std::shared_ptr<std::vector<uint8_t>> input_data;

    int packets_processed_in_tick;

    ServerConnection(std::string host, uint16_t port);

    bool connected{false};

    void join_server();

    void tick();
};

#endif // CAVEBLITZ_CLIENT_SERVER_CONNECTION_H
