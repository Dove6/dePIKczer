#include <iostream>
#include <limits>

#include "../external/piklib8_shim/include/piklib8_shim.h"
#include "exceptions.hpp"
#include "IMGFormat.hpp"

using namespace std;

uint32_t IMGColorDepth::get_value() const
{
    return value;
}

void IMGColorDepth::set_value(uint32_t new_value)
{
    switch (new_value) {
        case 2: //alias for 15
        case 4: //alias for 16
        case 8: //alias for 24
        case 15:
        case 16:
        case 24: {
            value = new_value;
            break;
        }
        default: {
            throw std::domain_error("IMGColorDepth: Inacceptable color depth value");
        }
    }
}

IMGColorDepth::IMGColorDepth(uint32_t value)
{
    set_value(value);
}

void IMGFormat::memory_decompress(const std::vector<unsigned char> &input_memory, PixelFormat pixel_format)
{
    //assert input_memory.size() >= 40
    uint32_t input_file_type = 0;
    unsigned char *binary_input_file_type = reinterpret_cast<unsigned char *>(&input_file_type);
    std::copy(input_memory.data(), input_memory.data() + sizeof(uint32_t), binary_input_file_type);
    if (input_file_type != IMGFormat::get_file_type()) {
        throw invalid_structure("IMGFormat: incorrect file type");
    }
    unsigned char *binary_width = reinterpret_cast<unsigned char *>(&width);
    unsigned char *binary_height = reinterpret_cast<unsigned char *>(&height);
    uint32_t input_color_depth = 0;
    unsigned char *binary_color_depth = reinterpret_cast<unsigned char *>(&input_color_depth);
    unsigned char *binary_image_size = reinterpret_cast<unsigned char *>(&image_size);
    uint32_t input_compression = 0;
    unsigned char *binary_compression = reinterpret_cast<unsigned char *>(&input_compression);
    unsigned char *binary_alpha_size = reinterpret_cast<unsigned char *>(&alpha_size);
    unsigned char *binary_x_position = reinterpret_cast<unsigned char *>(&x_position);
    unsigned char *binary_y_position = reinterpret_cast<unsigned char *>(&y_position);
    std::copy(input_memory.data() + sizeof(uint32_t), input_memory.data() + sizeof(uint32_t) * 2, binary_width);
    std::copy(input_memory.data() + sizeof(uint32_t) * 2, input_memory.data() + sizeof(uint32_t) * 3, binary_height);
    std::copy(input_memory.data() + sizeof(uint32_t) * 3, input_memory.data() + sizeof(uint32_t) * 4, binary_color_depth);
    std::copy(input_memory.data() + sizeof(uint32_t) * 4, input_memory.data() + sizeof(uint32_t) * 6, binary_image_size);
    std::copy(input_memory.data() + sizeof(uint32_t) * 6, input_memory.data() + sizeof(uint32_t) * 7, binary_compression);
    std::copy(input_memory.data() + sizeof(uint32_t) * 7, input_memory.data() + sizeof(uint32_t) * 8, binary_alpha_size);
    std::copy(input_memory.data() + sizeof(uint32_t) * 8, input_memory.data() + sizeof(uint32_t) * 9, binary_x_position);
    std::copy(input_memory.data() + sizeof(uint32_t) * 9, input_memory.data() + sizeof(uint32_t) * 10, binary_y_position);
    color_depth = IMGColorDepth(input_color_depth);
    compression = IMGCompression(input_compression);
}

void IMGFormat::memory_compress(std::vector<unsigned char> &output_memory)
{
    throw not_implemented();
}

constexpr uint32_t IMGFormat::get_file_type()
{
    return ('P' + ('I' << 8) + ('K' << 16));
}

uint32_t IMGFormat::get_width() const
{
    return width;
}

uint32_t IMGFormat::get_height() const
{
    return height;
}

IMGColorDepth IMGFormat::get_color_depth() const
{
    return color_depth;
}

void IMGFormat::set_color_depth(IMGColorDepth new_color_depth)
{
    color_depth = new_color_depth;
}

uint64_t IMGFormat::get_image_size() const
{
    return image_size;
}

IMGCompression IMGFormat::get_compression() const
{
    return compression;
}

void IMGFormat::set_compression(IMGCompression new_compression)
{
    compression = new_compression;
}

uint32_t IMGFormat::get_alpha_size() const
{
    return alpha_size;
}

uint32_t IMGFormat::get_x_position() const
{
    return x_position;
}

void IMGFormat::set_x_position(uint32_t new_x_position)
{
    x_position = new_x_position;
}

uint32_t IMGFormat::get_y_position() const
{
    return y_position;
}

void IMGFormat::set_y_position(uint32_t new_y_position)
{
    y_position = new_y_position;
}


const RawPixmap &IMGFormat::get_pixmap() const
{
    return pixmap;
}


IMGFormat::IMGFormat(const RawPixmap &pixmap)
: pixmap(pixmap)
{}

IMGFormat::IMGFormat(RawPixmap &&pixmap)
: pixmap(pixmap)
{}

IMGFormat::IMGFormat(const std::vector<unsigned char> &memory, PixelFormat pixel_format)
{
    memory_decompress(memory, pixel_format);
}

IMGFormat::IMGFormat(const std::string &filename, PixelFormat pixel_format)
{
    #ifdef _GLIBCXX_FILESYSTEM
        std::filesystem::path file_path = filename;
        if (std::filesystem::exists(file_path)) {
            uintmax_t file_size = std::filesystem::file_size(file_path);
            if (file_size < 40) {
                throw std::runtime_error("IMGFormat: specified file is too short");
            }
            std::ifstream file_handle(file_path);
            if (!file_handle) {
                throw std::runtime_error("IMGFormat: cannot open the file");
            }
            std::vector<unsigned char> file_content(file_size);
            file_handle.read(file_content.data(), file_content.size());
            if (!file_handle) {
                throw std::runtime_error("IMGFormat: error reading file content");
            }
            memory_decompress(file_content, pixel_format);
        } else {
            throw std::runtime_error("IMGFormat: cannot find the specified file");
        }
    #else
        std::ifstream file_handle(filename);
        if (!file_handle) {
            throw std::runtime_error("IMGFormat: cannot open the file");
        }
        //source: https://stackoverflow.com/a/22986486/7447673
        file_handle.ignore(std::numeric_limits<std::streamsize>::max());
        std::streamsize file_size = file_handle.gcount();
        file_handle.clear();
        file_handle.seekg(0, std::ios_base::beg);
        if (file_size < 40) {
            throw std::runtime_error("IMGFormat: specified file is too short");
        }
        std::vector<unsigned char> file_content(file_size);
        file_handle.read(reinterpret_cast<std::ifstream::char_type *>(file_content.data()), file_content.size());
        if (!file_handle) {
            throw std::runtime_error("IMGFormat: error reading file content");
        }
        memory_decompress(file_content, pixel_format);
    #endif // _GLIBCXX_FILESYSTEM
}

ostream &operator<<(ostream &os, const IMGHeader &ih)
{
	os << "[IMG header]\n"
        "  type: " << ih.file_type << "\n"
        "  width: " << ih.width << "px\n"
        "  height: " << ih.height << "px\n"
        "  depth: " << ih.color_depth << "bpp\n"
        "  image size (color): " << ih.image_size << "B\n"
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
