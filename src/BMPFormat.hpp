#ifndef DEPIKCZER_BMPFORMAT_HPP
#define DEPIKCZER_BMPFORMAT_HPP

#include <fstream>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "IMGFormat.hpp"

#include <pshpack2.h>
struct BITMAPHEADER {
    BITMAPFILEHEADER bf;
    BITMAPV5HEADER bV5;
};
#include <poppack.h>

BITMAPHEADER *prepare_bmp_header(const IMGHeader &img_header, const std::vector<unsigned char> &bmp_data);

std::vector<unsigned char> prepare_bmp_data(IMGHeader &img_header, std::vector<unsigned char> &img_data_color, std::vector<unsigned char> &img_data_alpha);

void write_bmp(std::ofstream &bmp_file, const IMGHeader &img_header, const std::vector<unsigned char> &bmp_data);

#endif // DEPIKCZER_BMPFORMAT_HPP
