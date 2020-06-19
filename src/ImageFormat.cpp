#include <stdexcept>

#include "ImageFormat.hpp"

void RawPixmap::check_correctness()
{
    unsigned bytes_per_pixel;
    switch (pixel_format) {
        case PixelFormat::RGB555:
        case PixelFormat::RGB565: {
            bytes_per_pixel = 2;
            break;
        }
        case PixelFormat::RGB24: {
            bytes_per_pixel = 3;
            break;
        }
        case PixelFormat::RGBA32: {
            bytes_per_pixel = 4;
            break;
        }
        default: {
            bytes_per_pixel = 0;
        }
    }
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

