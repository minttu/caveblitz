#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#include <png.h>

#include "Image.h"

Image read_png(const std::string &file_name) {
    Image image{};
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    FILE *fp = nullptr;

    try {
        fp = fopen(file_name.c_str(), "rb");
        if (fp == nullptr) {
            throw std::runtime_error("no such file");
        }

        unsigned char sig[8];
        fread(&sig, 1, 8, fp);
        if (!png_check_sig(png_bytep(sig), 8)) {
            throw std::runtime_error("not a valid png file");
        }

        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png_ptr == nullptr) {
            throw std::runtime_error("png read struct allocation failed");
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == nullptr) {
            throw std::runtime_error("png info struct allocation failed");
        }

        if (setjmp(png_jmpbuf(png_ptr))) { // NOLINT
            throw std::runtime_error("libpng error");
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        int bit_depth;
        int color_type;
        png_uint_32 width;
        png_uint_32 height;

        png_get_IHDR(png_ptr,
                     info_ptr,
                     &width,
                     &height,
                     &bit_depth,
                     &color_type,
                     nullptr,
                     nullptr,
                     nullptr);

        image.width = width;
        image.height = height;

        if (color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png_ptr);
        }

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) > 0) {
            png_set_tRNS_to_alpha(png_ptr);
        }

        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
            png_set_expand_gray_1_2_4_to_8(png_ptr);
        }

        if (color_type == PNG_COLOR_TYPE_RGB) {
            png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
        }

        png_set_strip_16(png_ptr);

        png_read_update_info(png_ptr, info_ptr);

        std::vector<png_bytep> row_pointers;
        row_pointers.reserve(height);

        for (uint32_t row = 0; row < height; row++) {
            row_pointers[row] = static_cast<png_bytep>(
                    png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr)));
        }

        png_read_image(png_ptr, row_pointers.data());

        for (uint32_t i = 0; i < height; ++i) {
            for (uint32_t j = 0; j < width * 4; j += 4) {
                const uint8_t r = row_pointers[i][j];     // NOLINT
                const uint8_t g = row_pointers[i][j + 1]; // NOLINT
                const uint8_t b = row_pointers[i][j + 2]; // NOLINT
                const uint8_t a = row_pointers[i][j + 3]; // NOLINT
                image.data.push_back({{r, g, b, a}});
            }
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
    } catch (const std::runtime_error &e) {
        if (png_ptr != nullptr) {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        }
        if (fp != nullptr) {
            fclose(fp);
        }
        throw;
    }

    return image;
}
