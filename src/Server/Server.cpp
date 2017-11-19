#include "Server.h"

Server::Server(uint16_t port)
        : update_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())),
          input_data(std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>())) {
    this->update_data->reserve(1024);
    this->input_data->reserve(1024);

    this->match = nullptr;

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

    std::cerr << "SERVER listening on 0.0.0.0:" << std::to_string(port) << "\n";
}

void Server::next_match() {
    std::cerr << "SERVER Loading match\n";
    this->match = std::make_shared<MatchServer>(MatchServer());
    this->match_version++;
}

void Server::run(const bool *should_run) {
    float dt = 0;
    ENetEvent event{};

    auto last_time = std::chrono::high_resolution_clock::now();
    auto start_time = last_time;
    std::chrono::duration<double, std::ratio<1, 120>> tick_duration(1);
    uint32_t ticks = 0;
    std::vector<PlayerID> joined_players;
    MatchReset match_reset;
    int connected_peers = 0;

    while (*should_run) {
        if (match != nullptr && match->match_status == MATCH_ENDED) {
            this->next_match();
        }

        while (enet_host_service(this->server, &event, 0) > 0) {
            auto peer_info = static_cast<PeerInfo *>(event.peer->data);
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                if (connected_peers == 0) {
                    std::cerr << "SERVER First peer connected, starting match\n";
                    this->next_match();
                }
                event.peer->data = new PeerInfo{this->match_version};
                connected_peers++;
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                delete static_cast<PeerInfo *>(event.peer->data);
                event.peer->data = nullptr;
                connected_peers--;
                if (connected_peers == 0) {
                    std::cerr << "SERVER Last peer disconnected, stopping match\n";
                    this->match = nullptr;
                }
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                if (peer_info->match_version != this->match_version) {
                    peer_info->match_version = this->match_version;
                    this->update_data->clear();

                    match_reset.serialize(this->update_data);
                    for (auto const &id : peer_info->players) {
                        this->match->join_player_id(id, this->update_data);
                    }
                    ENetPacket *response_packet = enet_packet_create(this->update_data->data(),
                                                                     this->update_data->size(),
                                                                     ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 2, response_packet);
                }

                this->update_data->clear();
                this->input_data->assign(event.packet->data,
                                         event.packet->data + event.packet->dataLength);
                joined_players = this->match->handle_input(this->input_data, this->update_data);
                enet_packet_destroy(event.packet);
                if (!update_data->empty()) {
                    ENetPacket *response_packet = enet_packet_create(this->update_data->data(),
                                                                     this->update_data->size(),
                                                                     ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 2, response_packet);
                }

                if (!joined_players.empty()) {
                    for (auto const &player_id : joined_players) {
                        peer_info->players.push_back(player_id);
                    }
                }
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            default:
                break;
            }
        }

        if (this->match != nullptr) {
            this->match->update(dt);
            this->update_data->clear();
            this->match->serialize(this->update_data);
            ENetPacket *update_packet =
                    enet_packet_create(this->update_data->data(), this->update_data->size(), 0);
            enet_host_broadcast(this->server, 0, update_packet);

            this->update_data->clear();
            this->match->serialize_reliable(this->update_data);
            if (!this->update_data->empty()) {
                update_packet = enet_packet_create(this->update_data->data(),
                                                   this->update_data->size(),
                                                   ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(this->server, 1, update_packet);
            }

            enet_host_flush(this->server);
        }

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
            std::cerr << "SERVER no sleep\n";
            ticks = 0;
            start_time = last_time;
        }
    }

    std::cerr << "SERVER stopping\n";
}

Server::~Server() {
    if (this->server != nullptr) {
        enet_host_destroy(this->server);
    }
    enet_deinitialize();
}
