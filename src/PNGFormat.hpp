#ifndef DEPIKCZER_PNGFORMAT_HPP
#define DEPIKCZER_PNGFORMAT_HPP

#include <vector>

#include "IMGFormat.hpp"

std::vector<char> prepare_png_data(IMGHeader &img_header, std::vector<char> &img_data_color, std::vector<char> &img_data_alpha);

#endif // DEPIKCZER_PNGFORMAT_HPP
