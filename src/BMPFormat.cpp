#include <iostream>

#include "BMPFormat.hpp"
#include "console.hpp"
#include "exceptions.hpp"
#include "IMGFormat.hpp"
#include "JPGFormat.hpp"

using namespace std;

BITMAPHEADER *prepare_bmp_header(const IMGHeader &img_header, const vector<unsigned char> &bmp_data)
{
    BITMAPHEADER *bmp_header = new BITMAPHEADER;

    bmp_header->bf.bfType = 0x4D42;
    bmp_header->bf.bfReserved1 = 0;
    bmp_header->bf.bfReserved2 = 0;
    bmp_header->bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV5HEADER);

    bmp_header->bV5.bV5Size = sizeof(BITMAPV5HEADER);
    bmp_header->bV5.bV5Width = img_header.width;
    bmp_header->bV5.bV5Height = img_header.height * -1; //-1 for upside-down image
    bmp_header->bV5.bV5Planes = 1;
    bmp_header->bV5.bV5BitCount = 16;
	bmp_header->bV5.bV5Compression = BI_BITFIELDS;
    bmp_header->bV5.bV5SizeImage = bmp_data.size();
    bmp_header->bV5.bV5XPelsPerMeter = 2835;
    bmp_header->bV5.bV5YPelsPerMeter = 2835;
    bmp_header->bV5.bV5ClrUsed = 0;
    bmp_header->bV5.bV5ClrImportant = 0;
	switch (img_header.color_depth) {
		case 15:
		case 16: {
			bmp_header->bV5.bV5RedMask =   0xF800;
			bmp_header->bV5.bV5GreenMask = 0x07E0;
			bmp_header->bV5.bV5BlueMask =  0x001F;
			bmp_header->bV5.bV5AlphaMask = 0;
			break;
		}
		default: {
			delete bmp_header;
			throw runtime_error("Unknown color format!\n");
		}
	}

	if (img_header.alpha_size > 0) {
		bmp_header->bV5.bV5BitCount = 32;
		bmp_header->bV5.bV5RedMask =   0x000000FF;
		bmp_header->bV5.bV5GreenMask = 0x0000FF00;
		bmp_header->bV5.bV5BlueMask =  0x00FF0000;
		bmp_header->bV5.bV5AlphaMask = 0xFF000000;
	} else if (img_header.compression == 5) {
		bmp_header->bV5.bV5BitCount = 24;
		bmp_header->bV5.bV5Compression = BI_RGB;
	}
    bmp_header->bV5.bV5CSType = LCS_sRGB;
    bmp_header->bV5.bV5Endpoints = tagICEXYZTRIPLE();
    bmp_header->bV5.bV5GammaRed = 0;
    bmp_header->bV5.bV5GammaGreen = 0;
    bmp_header->bV5.bV5GammaBlue = 0;
    bmp_header->bV5.bV5Intent = LCS_GM_GRAPHICS;
    bmp_header->bV5.bV5ProfileData = 0;
    bmp_header->bV5.bV5ProfileSize = 0;
    bmp_header->bV5.bV5Reserved = 0;

    bmp_header->bf.bfSize = bmp_header->bf.bfOffBits + bmp_header->bV5.bV5SizeImage;
	cout << "BMP header size: " << bmp_header->bf.bfOffBits << "\n";

    return bmp_header;
}

vector<unsigned char> prepare_bmp_data(IMGHeader &img_header, vector<unsigned char> &img_data_color, vector<unsigned char> &img_data_alpha)
{
	vector<unsigned char> aligned_bmp_data(0);
	char *buffer = (char *)(img_data_color.data());
	unsigned buffer_size = img_data_color.size();
	if (img_header.alpha_size == 0) {
		if (img_header.compression == 5) {
			if (img_data_color.size() != img_header.width * img_header.height * 3) {
				throw invalid_size("Incorrect color image data size!");
			}
		}
		if (img_header.color_depth == 15) { //for 16bpp go straight to padding check
			buffer_size = img_data_color.size();
			buffer = new char[buffer_size];
			copy(img_data_color.begin(), img_data_color.end(), buffer);
			#ifdef DEBUG
				clog << "[log] Copied color image data buffer!\n";
			#endif
			unsigned short green;
			for (unsigned i = 0; i < buffer_size; i += 2) {
				green = (((unsigned char)(buffer[i + 1]) & 0x3) << 3) | (((unsigned char)(buffer[i]) & 0xE0) >> 5);
				green *= 63 / 31.;
				buffer[i + 1] <<= 1;
				buffer[i + 1] &= 0xF8;
				buffer[i] &= 0x1F;
				buffer[i + 1] |= (0x7 & (green >> 2));
				buffer[i] |= (0xE0 & (green << 3));
			}
			#ifdef DEBUG
				clog << "[log] Converted image color format!\n";
			#endif
			//go to padding check
		}
	} else {
		if (img_header.compression == 5) {
			if (img_data_color.size() != img_header.width * img_header.height * 4) {
				throw invalid_size("Incorrect color image data size!");
			}
		}
		unsigned pixel_count = img_header.width * img_header.height;
		if (img_data_color.size() / 2 == img_data_alpha.size()) {
			buffer_size = pixel_count * 4;
			buffer = new char[buffer_size];
			char pixel[4];
			if (img_header.color_depth == 16) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
					pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			} else if (img_header.color_depth == 15) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
					pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			}
		} else if (img_data_color.size() / 4 == img_data_alpha.size() && img_header.compression == 5) {
			for (unsigned i = 0; i < pixel_count; i++) {
				//after JPEG decompression of IMG with alpha img_data_color has "holes" every 4 bytes
				//and buffer points to img_data_color.data()
				//so basically we are filling these holes here
				buffer[i * 4 + 3] = img_data_alpha[i];
			}
		} else {
			img_header.alpha_size = 0;
			throw invalid_size("Unknown alpha data format!\n");
		}
	}

	if (buffer_size / img_header.height % 4 > 0) {
		unsigned row_length = buffer_size / img_header.height;
		unsigned padding_length = 4 - row_length % 4;
		unsigned padded_row_length = row_length + padding_length;
		unsigned padded_buffer_size = padded_row_length * img_header.height;
		char padding[4] = {0, 0, 0, 0};
		char *padded_buffer = new char[padded_buffer_size];
		for (unsigned i = 0; i < img_header.height; i++) {
			copy(buffer + i * row_length, buffer + (i + 1) * row_length, padded_buffer + i * padded_row_length);
			copy(padding, padding + padding_length, padded_buffer + i * padded_row_length + row_length);
		}
		#ifdef DEBUG
			clog << "[log] Converted image data!\n";
		#endif
		aligned_bmp_data.reserve(padded_buffer_size);
		aligned_bmp_data.assign(padded_buffer, padded_buffer + padded_buffer_size);
		delete[] padded_buffer;
	} else {
		#ifdef DEBUG
			clog << "[log] Converted image data!\n";
		#endif
		aligned_bmp_data.reserve(buffer_size);
		aligned_bmp_data.assign(buffer, buffer + buffer_size);
	}

	if (buffer != (char *)img_data_color.data()) {
		delete[] buffer;
	}
	return aligned_bmp_data;
}

void write_bmp(ofstream &bmp_file, const IMGHeader &img_header, const vector<unsigned char> &bmp_data)
{
	BITMAPHEADER *bmp_header = prepare_bmp_header(img_header, bmp_data);
	bmp_file.write((char *)(bmp_header), sizeof(BITMAPHEADER));
	bmp_file.write((const char *)bmp_data.data(), bmp_data.size());
}
