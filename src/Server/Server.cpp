#include "Server.h"

Server::Server()
        : update_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())),
          input_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())) {
    this->update_data->reserve(1024);
    this->input_data->reserve(1024);

    this->match = std::make_shared<MatchServer>(MatchServer());

    if (enet_initialize() != 0) {
        throw std::runtime_error("enet_initialize() failed");
    }

    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = 30320;

    this->server = enet_host_create(&address, 32, 3, 0, 0);
    if (this->server == nullptr) {
        enet_deinitialize();
        throw std::runtime_error("enet_host_create() failed");
    }

    std::cerr << "host created\n";
}

void Server::run(const bool *should_run) {
    float dt = 0;
    ENetEvent event{};

    while (*should_run) {
        while (enet_host_service(this->server, &event, 0) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                this->update_data->clear();
                this->input_data->assign(event.packet->data,
                                         event.packet->data + event.packet->dataLength);
                this->match->handle_input(this->input_data, this->update_data);
                enet_packet_destroy(event.packet);
                if (!update_data->empty()) {
                    ENetPacket *response_packet = enet_packet_create(this->update_data->data(),
                                                                     this->update_data->size(),
                                                                     ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 2, response_packet);
                }
                break;
            default:
                break;
            }
        }

        // update
        this->match->update(dt);
        this->update_data->clear();
        this->match->serialize(this->update_data);
        ENetPacket *update_packet =
                enet_packet_create(this->update_data->data(), this->update_data->size(), 0);
        enet_host_broadcast(this->server, 0, update_packet);

        this->update_data->clear();
        this->match->serialize_reliable(this->update_data);
        update_packet = enet_packet_create(this->update_data->data(),
                                           this->update_data->size(),
                                           ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(this->server, 1, update_packet);

        // send
        enet_host_flush(this->server);

        // sleep
        dt = 1.0f / 60.0f;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }
}

Server::~Server() {
    if (this->server != nullptr) {
        enet_host_destroy(this->server);
    }
    enet_deinitialize();
}
