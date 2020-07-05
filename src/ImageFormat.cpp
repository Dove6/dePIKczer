#include <stdexcept>

#include "ImageFormat.hpp"

unsigned get_pixel_size(PixelFormat pixel_format)
{
    switch (pixel_format) {
        case PixelFormat::RGB555:
        case PixelFormat::RGB565: {
            return 2;
        }
        case PixelFormat::RGB24: {
            return 3;
        }
        case PixelFormat::RGBA32: {
            return 4;
        }
        default: {
            return 0;
        }
    }
}

void RawPixmap::check_correctness()
{
    unsigned bytes_per_pixel = get_pixel_size(pixel_format);
    std::vector<unsigned char>::size_type required_size = width * height * bytes_per_pixel;
    if (pixel_data.size() < required_size) {
        throw std::logic_error("RawPixmap: insufficient pixel_data length");
    }
}

uint32_t RawPixmap::get_width() const
{
    return width;
}

uint32_t RawPixmap::get_height() const
{
    return height;
}

PixelFormat RawPixmap::get_pixel_format() const
{
    return pixel_format;
}

const std::vector<unsigned char> &RawPixmap::get_pixel_data() const
{
    return pixel_data;
}

RawPixmap::RawPixmap(uint32_t width, uint32_t height, PixelFormat pixel_format, const std::vector<unsigned char> &pixel_data)
: width(width), height(height), pixel_format(pixel_format), pixel_data(pixel_data)
{
    check_correctness();
}

RawPixmap::RawPixmap(uint32_t width, uint32_t height, PixelFormat pixel_format, std::vector<unsigned char> &&pixel_data)
: width(width), height(height), pixel_format(pixel_format), pixel_data(pixel_data)
{
    check_correctness();
}

