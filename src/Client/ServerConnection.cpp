#include "ServerConnection.h"

ServerConnection::ServerConnection()
        : update_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())),
          input_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())) {

    this->update_data->reserve(1024);
    this->input_data->reserve(1024);

    this->client = enet_host_create(nullptr, 1, 3, 0, 0);
    if (this->client == nullptr) {
        throw std::runtime_error("couldn't create enet client");
    }
    ENetAddress address{};
    enet_address_set_host(&address, "localhost");
    address.port = 30320;
    this->peer = enet_host_connect(this->client, &address, 3, 0);
    if (this->peer == nullptr) {
        throw std::runtime_error("couldn't create enet peer");
    }
}

void ServerConnection::tick() {
    this->update_data->clear();

    ENetEvent event{};
    while (enet_host_service(this->client, &event, 0) > 0) {
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
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            this->connected = false;
            std::cerr << "disconnected!\n";
            break;
        default:
            break;
        }
    }

    if (!this->connected) {
        return;
    }

    ENetPacket *packet = enet_packet_create(this->input_data->data(), this->input_data->size(), 0);

    enet_peer_send(this->peer, 0, packet);

    enet_host_flush(this->client);
}
