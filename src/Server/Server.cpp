#include "Server.h"

Server::Server(uint16_t port)
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
    address.port = port;

    this->server = enet_host_create(&address, 32, 3, 0, 0);
    if (this->server == nullptr) {
        enet_deinitialize();
        throw std::runtime_error("enet_host_create() failed");
    }

    std::cerr << "server listening on 0.0.0.0:" << std::to_string(port) << "\n";
}

void Server::run(const bool *should_run) {
    float dt = 0;
    ENetEvent event{};

    auto last_time = std::chrono::high_resolution_clock::now();
    auto start_time = last_time;
    std::chrono::duration<double, std::ratio<1, 120>> tick_duration(1);
    uint32_t ticks = 0;

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
        ticks++;
        auto current_time = std::chrono::high_resolution_clock::now();
        auto delta_time =
                std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_time);
        last_time = current_time;
        auto target_time = start_time + (ticks * tick_duration);

        dt = delta_time.count() / 1000.0f / 1000.0f;

        if (current_time <= target_time) {
            std::this_thread::sleep_for(target_time - current_time);
        } else {
            std::cerr << "server no sleep\n";
            ticks = 0;
            start_time = last_time;
        }
    }
}

Server::~Server() {
    if (this->server != nullptr) {
        enet_host_destroy(this->server);
    }
    enet_deinitialize();
}
