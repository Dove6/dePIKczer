#ifndef DEPIKCZER_JPGFORMAT_HPP
#define DEPIKCZER_JPGFORMAT_HPP

#include <vector>

#include "console.hpp"
#include "IMGFormat.hpp"

void compress_jpg(std::vector<unsigned char> &img_data_color, const IMGHeader &img_header);

void decompress_jpg(std::vector<unsigned char> &img_data_color, const IMGHeader &img_header, enum output_format format);

std::vector<unsigned char> prepare_jpg_data(const IMGHeader &img_header, std::vector<unsigned char> &img_data_color);

#endif // DEPIKCZER_JPGFORMAT_HPP
