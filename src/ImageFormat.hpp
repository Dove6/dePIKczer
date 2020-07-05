#ifndef DEPIKCZER_IMAGEFORMAT_HPP
#define DEPIKCZER_IMAGEFORMAT_HPP

#include <cstdint>
#include <vector>

enum class PixelFormat : unsigned {
    RGB555,
    RGB565,
    RGB24,
    RGBA32
};

unsigned get_pixel_size(PixelFormat pixel_format);

class RawPixmap {
    uint32_t width;
    uint32_t height;
    PixelFormat pixel_format;
    std::vector<unsigned char> pixel_data;

    void check_correctness();

public:
    uint32_t get_width() const;
    uint32_t get_height() const;
    PixelFormat get_pixel_format() const;
    const std::vector<unsigned char> &get_pixel_data() const;

    RawPixmap() = default;
    RawPixmap(uint32_t width, uint32_t height, PixelFormat pixel_format, const std::vector<unsigned char> &pixel_data);
    RawPixmap(uint32_t width, uint32_t height, PixelFormat pixel_format, std::vector<unsigned char> &&pixel_data);
};

#endif // DEPIKCZER_IMAGEFORMAT_HPP
