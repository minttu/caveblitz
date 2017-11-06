#ifndef CAVEBLITZ_SERVER_SERVER_PICKUP_H
#define CAVEBLITZ_SERVER_SERVER_PICKUP_H

#include <cstdint>

class ServerPickup {
public:
    uint8_t pickup_id;
    uint8_t pickup_type;
    uint16_t x;
    uint16_t y;

    ServerPickup(uint8_t pickup_id, uint8_t pickup_type, uint16_t x, uint16_t y);
};

#endif // CAVEBLITZ_SERVER_SERVER_PICKUP_H
