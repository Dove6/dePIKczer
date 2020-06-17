#include <iostream>

#include "../external/piklib8_shim/include/piklib8_shim.h"
#include "exceptions.hpp"
#include "IMGFormat.hpp"

using namespace std;

ostream &operator<<(ostream &os, const IMGHeader &ih)
{
	os << "[IMG header]\n"
        "  type: " << ih.file_type << "\n"
        "  width: " << ih.width << "px\n"
        "  height: " << ih.height << "px\n"
        "  depth: " << ih.color_depth << "bpp\n"
        "  image size (color): " << ih.image_size << "B\n"
        "  padding: " << ih.reserved << "\n"
        "  compression: " << ih.compression
            << (ih.compression == 0 ? " (none)" :
                (ih.compression == 4 ? " (unspecified)" :
                (ih.compression == 2 ? " (CLZW2)" :
                (ih.compression == 5 ? " (JPEG)" : "")))) << "\n"
        "  image size (alpha): " << ih.alpha_size << "B\n"
        "  X position: " << ih.x_position << "px\n"
        "  Y position: " << ih.y_position << "px\n";
	return os;
}

IMGHeader read_img_header(ifstream &img_file)
{
    iostream::pos_type init_pos;
	init_pos = img_file.tellg();
	img_file.seekg(0, ios::beg);

    IMGHeader img_header;
	try {
		img_file.read((char *)(&img_header), sizeof(IMGHeader));
	} catch (const ifstream::failure &) {
		throw io_failure("Error reading image header!");
	}

	cout << "Read IMG header!\n";

	img_file.seekg(init_pos);
    return img_header;
}

void check_img_header(const IMGHeader &img_header)
{
	if (img_header.file_type != '\0KIP') {
		throw invalid_structure("Incorrect header identifier!");
	}
	if (img_header.color_depth != 15 && img_header.color_depth != 16) {
		throw invalid_structure("Unknown color format!");
	}
	if (img_header.compression != 0 && img_header.compression != 2 && img_header.compression != 4 && img_header.compression != 5) {
		throw invalid_structure("Unknown compression!");
	}
	if (img_header.reserved != 0) {
		throw invalid_structure("Unexpected padding value!");
	}
	cout << "The header is correct!\n";
}

void read_img_data(ifstream &img_file, IMGHeader &img_header, vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	iostream::pos_type init_pos;
	init_pos = img_file.tellg();
	img_file.seekg(40, ios::beg);

	char *buffer;
	buffer = new char[img_header.image_size];
	try {
		img_file.read(buffer, img_header.image_size);
	} catch (ifstream::failure &) {
        img_header.alpha_size = 0;
		for (streamsize i = img_file.gcount(); i < int(img_header.image_size); i++) {
			buffer[i] = 0;
		}
		img_data_color.assign(buffer, buffer + img_header.image_size);
		delete[] buffer;
		throw io_failure("Error reading color image data! (padded with zeroes)");
    }
	img_data_color.assign(buffer, buffer + img_header.image_size);
	delete[] buffer;

	if (img_header.alpha_size != 0) {
		buffer = new char[img_header.alpha_size];
		try {
			img_file.read(buffer, img_header.alpha_size);
			img_data_alpha.assign(buffer, buffer + img_header.alpha_size);
		} catch (ifstream::failure &) {
			img_header.alpha_size = 0;
			delete[] buffer;
			throw io_failure("Error reading alpha image data! (skipped)");
		}
		delete[] buffer;
	}

    img_file.seekg(init_pos);
}

void determine_compression_format(IMGHeader &img_header, vector<char> &img_data_color)
{
	if (img_header.width * img_header.height * 2 == img_header.image_size) {
		img_header.compression = 0;
	} else {
		string test(img_data_color.data(), 4);
		if (test == string("\xFF\xD8\xFF\xE0")) {
			img_header.compression = 5;
		} else {
			test.assign(img_data_color.end() - 3, img_data_color.end());
			if (test == string("\x11\x00\x00")) {
				img_header.compression = 2;
			} else {
				throw invalid_structure("Unknown compression!");
			}
		}
	}
}

void decompress_img(vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	char *buffer;
	int size;

	if (img_data_color.size() > 0) {
		buffer = piklib_CLZWCompression2_decompress(img_data_color.data(), img_data_color.size());
		size = reinterpret_cast<int *>(img_data_color.data())[0];
		img_data_color.reserve(size);
		img_data_color.assign(buffer, buffer + size);
	}

	if (img_data_alpha.size() > 0) {
		buffer = piklib_CLZWCompression2_decompress(img_data_alpha.data(), img_data_alpha.size());
		size = reinterpret_cast<int *>(img_data_alpha.data())[0];
		img_data_alpha.reserve(size);
		img_data_alpha.assign(buffer, buffer + size);
	}
}
