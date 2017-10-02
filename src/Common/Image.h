#ifndef CAVEBLITZ_COMMON_IMAGE_H
#define CAVEBLITZ_COMMON_IMAGE_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct Image {
    uint32_t width;
    uint32_t height;
    std::vector<std::array<uint8_t, 4>> data;

    std::array<uint8_t, 4> at(uint32_t x, uint32_t y) {
        return data[x + (y * width)];
    }
};

Image read_png(const std::string &file_name);

#endif // CAVEBLITZ_COMMON_IMAGE_H
