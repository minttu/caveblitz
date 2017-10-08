#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

#include <enet/enet.h>

#include "Server/Server.h"

int main() {
    Server game_server;

    if (enet_initialize() != 0) {
        std::cerr << "enet initialization failed\n";
        return 1;
    }

    ENetAddress address{};
    ENetHost *server;

    address.host = ENET_HOST_ANY;
    address.port = 30320;

    server = enet_host_create(&address, 32, 3, 0, 0);
    if (server == nullptr) {
        std::cerr << "enet host creation failed\n";
        enet_deinitialize();
        return 1;
    }

    std::cerr << "host created\n";

    auto update_data = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>());
    update_data->reserve(1024);
    auto input_data = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>());
    input_data->reserve(1024);

    float dt = 0;
    ENetEvent event{};
    while (true) {
        // gett
        while (enet_host_service(server, &event, 0) > 0) {
            switch(event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                update_data->clear();
                input_data->assign(event.packet->data, event.packet->data + event.packet->dataLength);
                game_server.handle_input(input_data, update_data);
                enet_packet_destroy(event.packet);
                if (!update_data->empty()) {
                    ENetPacket *response_packet = enet_packet_create(
                        update_data->data(),
                        update_data->size(),
                        ENET_PACKET_FLAG_RELIABLE
                    );
                    enet_peer_send(event.peer, 2, response_packet);
                }
                break;
            default:
                break;
            }
        }

        // update
        game_server.update(dt);
        update_data->clear();
        game_server.serialize(update_data);

        ENetPacket *update_packet = enet_packet_create(
            update_data->data(),
            update_data->size(),
            0
        );

        // sendd
        enet_host_broadcast(server, 0, update_packet);
        enet_host_flush(server);

        // sleep
        dt = 1.0f / 60.0f;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    enet_host_destroy(server);
    enet_deinitialize();
    return 0;
}
