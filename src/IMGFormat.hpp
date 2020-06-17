#ifndef DEPIKCZER_IMGFORMAT_HPP
#define DEPIKCZER_IMGFORMAT_HPP

#include <cstdint>
#include <fstream>
#include <vector>

struct IMGHeader {
    uint32_t file_type;
    uint32_t width;
    uint32_t height;
    uint32_t color_depth;
    uint32_t image_size;
    uint32_t reserved;
    uint32_t compression;
    uint32_t alpha_size;
    int32_t x_position;
    int32_t y_position;
};

std::ostream &operator<<(std::ostream &os, const IMGHeader &ih);

IMGHeader read_img_header(std::ifstream &img_file);

void check_img_header(const IMGHeader &img_header);

void read_img_data(std::ifstream &img_file, IMGHeader &img_header, std::vector<char> &img_data_color, std::vector<char> &img_data_alpha);

void determine_compression_format(IMGHeader &img_header, std::vector<char> &img_data_color);

void decompress_img(std::vector<char> &img_data_color, std::vector<char> &img_data_alpha);

#endif // DEPIKCZER_IMGFORMAT_HPP
