#include "../external/libjpeg-turbo/turbojpeg.h"
#include "console.hpp"
#include "exceptions.hpp"
#include "JPGFormat.hpp"

using namespace std;

void compress_jpg(vector<unsigned char> &img_data_color, const IMGHeader &img_header)
{
	tjhandle compressor = tjInitCompress();
	if (compressor != nullptr) {
		unsigned long buffer_size = tjBufSize(img_header.width, img_header.height, TJSAMP_444);
		char *buffer = new char[buffer_size];
		if (!tjCompress2(compressor, (const unsigned char *)(img_data_color.data()), img_header.width, 0, img_header.height,
						 TJPF_RGB, (unsigned char **)&buffer, &buffer_size, TJSAMP_444, 90, TJFLAG_NOREALLOC)) {
			img_data_color.assign(buffer, buffer + buffer_size);
			tjDestroy(compressor);
		} else {
			tjDestroy(compressor);
			throw compression_failure(tjGetErrorStr2(compressor));
		}
	} else {
		throw compression_failure(tjGetErrorStr2(compressor));
	}
}

void decompress_jpg(vector<unsigned char> &img_data_color, const IMGHeader &img_header, enum output_format format)
{
	tjhandle decompressor = tjInitDecompress();
	if (decompressor != nullptr) {
		unsigned buffer_size = img_header.width * img_header.height * (img_header.alpha_size > 0 ? 4 : 3);
		char *buffer = new char[buffer_size];
		int pixel_format;
		if (img_header.alpha_size > 0) {
			pixel_format = TJPF_RGBA;
		} else {
			if (format == BMP) {
				pixel_format = TJPF_BGR;
			} else {
				pixel_format = TJPF_RGB;
			}
		}
		if (!tjDecompress2(decompressor, (const unsigned char *)(img_data_color.data()), img_data_color.size(), (unsigned char *)(buffer),
						  img_header.width, img_header.width * (img_header.alpha_size > 0 ? 4 : 3), img_header.height,
						  pixel_format, 0)) {
			img_data_color.assign(buffer, buffer + buffer_size);
			tjDestroy(decompressor);
		} else {
			tjDestroy(decompressor);
			throw compression_failure(tjGetErrorStr2(decompressor));
		}
	} else {
		throw compression_failure(tjGetErrorStr2(decompressor));
	}
}

vector<unsigned char> prepare_jpg_data(const IMGHeader &img_header, vector<unsigned char> &img_data_color)
{
    if (img_data_color.size() != img_header.width * img_header.height * 2) {
        throw invalid_size("Incorrect color image data size!");
    }
    unsigned pixel_count = img_header.width * img_header.height;
    unsigned buffer_size = pixel_count * 3;
    char *buffer = new char[buffer_size];
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
    img_data_color.assign(buffer, buffer + buffer_size);
    delete[] buffer;
    compress_jpg(img_data_color, img_header);
    return img_data_color;
}
