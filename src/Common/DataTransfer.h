#ifndef CAVEBLITZ_COMMON_DATA_TRANSFER_H
#define CAVEBLITZ_COMMON_DATA_TRANSFER_H

#include <gsl/gsl>
#include <memory>
#include <vector>

union Bytes4 {
    float f;
    uint32_t i;
    uint8_t b[4];

    inline void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(b[0]);
        target->push_back(b[1]);
        target->push_back(b[2]);
        target->push_back(b[3]);
    }

    inline void deserialize(const gsl::span<uint8_t> &target, uint32_t offset) {
        memcpy(&b, target.data() + offset, 4);
    }
};

union Bytes2 {
    uint16_t i;
    uint8_t b[2];

    inline void serialize(const std::shared_ptr<std::vector<uint8_t>> &target) const {
        target->push_back(b[0]);
        target->push_back(b[1]);
    }

    inline void deserialize(const gsl::span<uint8_t> &target, uint32_t offset) {
        memcpy(&b, target.data() + offset, 2);
    }
};

using PlayerID = uint8_t;
using ProjectileID = uint16_t;
using PickupID = uint8_t;
using PickupType = uint8_t;

#endif // CAVEBLITZ_COMMON_DATA_TRANSFER_H
