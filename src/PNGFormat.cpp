#include "../external/miniz/miniz.h"
#include "console.hpp"
#include "exceptions.hpp"
#include "JPGFormat.hpp"
#include "PNGFormat.hpp"

using namespace std;

vector<unsigned char> prepare_png_data(IMGHeader &img_header, vector<unsigned char> &img_data_color, vector<unsigned char> &img_data_alpha)
{
	vector<unsigned char> png_data(0);
    if (img_data_color.size() != img_header.width * img_header.height * 2) {
        throw invalid_size("Incorrect color image data size!");
    }
    unsigned pixel_count = img_header.width * img_header.height;
    size_t buffer_size = pixel_count * (img_header.alpha_size > 0 ? 4 : 3);
    char *buffer = new char[buffer_size];
    if (img_header.alpha_size == 0) {
        char pixel[3];
        if (img_header.color_depth == 15) {
            for (unsigned i = 0; i < pixel_count; i++) {
                pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
                pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
                pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
                copy(pixel, pixel + 3, buffer + i * 3);
            }
        } else if (img_header.color_depth == 16) {
            for (unsigned i = 0; i < pixel_count; i++) {
                pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
                pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
                pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
                copy(pixel, pixel + 3, buffer + i * 3);
            }
        }
    } else if (img_data_color.size() / 2 == img_data_alpha.size()) {
        char pixel[4];
        if (img_header.color_depth == 15) {
            for (unsigned i = 0; i < pixel_count; i++) {
                pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
                pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
                pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
                pixel[3] = img_data_alpha[i]; //alpha
                copy(pixel, pixel + 4, buffer + i * 4);
            }
        } else if (img_header.color_depth == 16) {
            for (unsigned i = 0; i < pixel_count; i++) {
                pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
                pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
                pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
                pixel[3] = img_data_alpha[i]; //alpha
                copy(pixel, pixel + 4, buffer + i * 4);
            }
        }
    } else {
        img_header.alpha_size = 0;
        throw runtime_error("Unknown alpha data format!\n");
    }
    img_data_color.assign(buffer, buffer + buffer_size);
    delete[] buffer;
    buffer_size = 0;
	buffer = (char *)tdefl_write_image_to_png_file_in_memory_ex(img_data_color.data(), img_header.width, img_header.height,
        (img_header.alpha_size > 0 ? 4 : 3), &buffer_size, MZ_BEST_COMPRESSION, false);
	if (buffer != nullptr && buffer_size != 0) {
		png_data.assign(buffer, buffer + buffer_size);
		mz_free(buffer);
	} else {
		throw compression_failure("Error converting image data to PNG format!\n");
	}
	return png_data;
}
