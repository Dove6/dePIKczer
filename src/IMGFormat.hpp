#ifndef DEPIKCZER_IMGFORMAT_HPP
#define DEPIKCZER_IMGFORMAT_HPP

#include <cstdint>
#include <fstream>
#include <vector>

#include "ImageFormat.hpp"

struct IMGHeader {
    uint32_t file_type;
    uint32_t width;
    uint32_t height;
    uint32_t color_depth;
    uint64_t image_size;
    uint32_t compression;
    uint32_t alpha_size;
    int32_t x_position;
    int32_t y_position;
};

enum class IMGCompression : uint32_t {
    NONE_NONE = 0,
    CLZW2_CLZW2 = 2,
    UNKNOWN = 4,
    JPEG_CLZW2 = 5
};

class IMGColorDepth {
    uint32_t value;

public:
    uint32_t get_value() const;
    void set_value(uint32_t new_value);

    IMGColorDepth(uint32_t value = 16);
};

class IMGFormat {
    uint32_t width;
    uint32_t height;
    IMGColorDepth color_depth;
    uint64_t image_size;
    IMGCompression compression;
    uint32_t alpha_size;
    int32_t x_position;
    int32_t y_position;

    RawPixmap pixmap;

    void memory_decompress(const std::vector<unsigned char> &input_memory, PixelFormat pixel_format);
    void memory_compress(std::vector<unsigned char> &output_memory);

public:
    static constexpr uint32_t get_file_type();
    uint32_t get_width() const;
    uint32_t get_height() const;
    IMGColorDepth get_color_depth() const;
    void set_color_depth(IMGColorDepth new_color_depth);
    uint64_t get_image_size() const;
    IMGCompression get_compression() const;
    void set_compression(IMGCompression new_compression);
    uint32_t get_alpha_size() const;
    uint32_t get_x_position() const;
    void set_x_position(uint32_t new_x_position);
    uint32_t get_y_position() const;
    void set_y_position(uint32_t new_y_position);

    const RawPixmap &get_pixmap() const;

    IMGFormat(const RawPixmap &pixmap);
    IMGFormat(RawPixmap &&pixmap);
    IMGFormat(const std::vector<unsigned char> &memory, PixelFormat pixel_format = PixelFormat::RGBA32);
    IMGFormat(const std::string &filename, PixelFormat pixel_format = PixelFormat::RGBA32);
};

std::ostream &operator<<(std::ostream &os, const IMGHeader &ih);

IMGHeader read_img_header(std::ifstream &img_file);

void check_img_header(const IMGHeader &img_header);

void read_img_data(std::ifstream &img_file, IMGHeader &img_header, std::vector<char> &img_data_color, std::vector<char> &img_data_alpha);

void determine_compression_format(IMGHeader &img_header, std::vector<char> &img_data_color);

void decompress_img(std::vector<char> &img_data_color, std::vector<char> &img_data_alpha);

#endif // DEPIKCZER_IMGFORMAT_HPP
