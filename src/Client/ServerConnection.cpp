#include "ServerConnection.h"

ServerConnection::ServerConnection(const std::string host, int port)
        : update_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())),
          input_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())) {

    this->update_data->reserve(1024);
    this->input_data->reserve(1024);

    this->packets_processed_in_tick = 0;

    this->client = enet_host_create(nullptr, 1, 3, 0, 0);
    if (this->client == nullptr) {
        throw std::runtime_error("couldn't create enet client");
    }
    ENetAddress address{};
    enet_address_set_host(&address, host.c_str());
    address.port = port;
    this->peer = enet_host_connect(this->client, &address, 3, 0);
    if (this->peer == nullptr) {
        throw std::runtime_error("couldn't create enet peer");
    }
}

void ServerConnection::tick() {
    this->update_data->clear();

    if (this->connected) {
        ENetPacket *packet =
                enet_packet_create(this->input_data->data(), this->input_data->size(), 0);

        enet_peer_send(this->peer, 0, packet);

        enet_host_flush(this->client);
    }

    this->packets_processed_in_tick = 0;
    int wait = 10;

    ENetEvent event{};
    while (enet_host_service(this->client, &event, wait) > 0) {
        wait = 0;

        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            this->connected = true;
            std::cerr << "connected!\n";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            this->update_data->insert(this->update_data->end(),
                                      event.packet->data,
                                      event.packet->data + event.packet->dataLength); // NOLINT
            enet_packet_destroy(event.packet);
            this->packets_processed_in_tick++;
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            this->connected = false;
            std::cerr << "disconnected!\n";
            break;
        default:
            break;
        }
    }
}

void ServerConnection::join_server() {
    if (!this->connected) {
        return;
    }

    auto join_data = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>());
    PlayerJoinServer join{};
    join.serialize(join_data);

    ENetPacket *packet =
            enet_packet_create(join_data->data(), join_data->size(), ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(this->peer, 2, packet);

    enet_host_flush(this->client);
}
