#ifndef DEPIKCZER_IMGFORMAT_HPP
#define DEPIKCZER_IMGFORMAT_HPP

#include <cstdint>
#include <fstream>
#include <vector>

#include "ImageFormat.hpp"

class IMGColorDepth {
public:
    enum Enum : uint32_t {
        RGB555ALIAS = 2,
        RGB565ALIAS = 4,
        RGB888ALIAS = 8,
        RGB555 = 15,
        RGB565 = 16,
        RGB888 = 24
    };

private:
    Enum value;
    void set_value(uint32_t new_value);

public:
    operator uint32_t() const;
    std::size_t get_pixel_size() const;

    IMGColorDepth(uint32_t value = 16);
};

class IMGCompression {
public:
    enum Enum : uint32_t {
        NONE_NONE = 0,
        CLZW2_CLZW2 = 2,
        UNKNOWN = 4,
        JPEG_CLZW2 = 5
    };

private:
    Enum value;
    void set_value(uint32_t new_value);

public:
    operator uint32_t() const;

    IMGCompression(uint32_t value = 0);
};

struct IMGHeader {
    uint32_t width;
    uint32_t height;
    IMGColorDepth color_depth;
    uint64_t image_size;
    IMGCompression compression;
    uint32_t alpha_size;
    int32_t x_position;
    int32_t y_position;
};


class IMGFormat {
    IMGHeader header;

    std::vector<unsigned char> uncompressed_rgb_data;
    std::vector<unsigned char> uncompressed_alpha_data;

    void memory_decompress(const std::vector<unsigned char> &input_memory);
    void memory_compress(std::vector<unsigned char> &output_memory);

public:
    const IMGHeader &get_header() const;

    const std::vector<unsigned char> &get_pixmap_as_rgb555();
    const std::vector<unsigned char> &get_pixmap_as_rgb565();
    const std::vector<unsigned char> &get_pixmap_as_rgb888();
    const std::vector<unsigned char> &get_pixmap_as_rgba8888();

    const std::vector<unsigned char> &get_original_rgb_pixmap();
    const std::vector<unsigned char> &get_original_alpha_pixmap();

    static IMGFormat from_rgb555_pixmap(const std::vector<unsigned char> &pixmap, const IMGHeader &header);
    static IMGFormat from_rgb565_pixmap(const std::vector<unsigned char> &pixmap, const IMGHeader &header);
    static IMGFormat from_rgb888_pixmap(const std::vector<unsigned char> &pixmap, const IMGHeader &header);
    static IMGFormat from_rgba8888_pixmap(const std::vector<unsigned char> &pixmap, const IMGHeader &header);

    IMGFormat(const std::vector<unsigned char> &memory);
    IMGFormat(const std::string &filename);
};

std::ostream &operator<<(std::ostream &os, const IMGHeader &ih);

IMGHeader read_img_header(std::ifstream &img_file);

void check_img_header(const IMGHeader &img_header);

void read_img_data(std::ifstream &img_file, IMGHeader &img_header, std::vector<char> &img_data_color, std::vector<char> &img_data_alpha);

void determine_compression_format(IMGHeader &img_header, std::vector<unsigned char> &img_data_color);

void decompress_img(std::vector<char> &img_data_color, std::vector<char> &img_data_alpha);

#endif // DEPIKCZER_IMGFORMAT_HPP
