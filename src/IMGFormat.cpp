#include <array>
#include <iostream>
#include <iterator>
#include <limits>
#include <type_traits>

#include "../external/piklib8_shim/include/piklib8_shim.h"
#include "exceptions.hpp"
#include "IMGFormat.hpp"

using namespace std;


void IMGCompression::set_value(uint32_t new_value)
{
    switch (Enum(new_value)) {
        case NONE_NONE:
        case CLZW2_CLZW2:
        case UNKNOWN:
        case JPEG_CLZW2: {
            value = Enum(new_value);
            break;
        }
        default: {
            throw std::domain_error("IMGCompression: Undefined compression type");
        }
    }
}

IMGCompression::operator uint32_t() const
{
    return uint32_t(value);
}

IMGCompression::IMGCompression(uint32_t value)
{
    set_value(value);
}

IMGColorDepth::operator uint32_t() const
{
    return uint32_t(value);
}

void IMGColorDepth::set_value(uint32_t new_value)
{
    switch (Enum(new_value)) {
        case RGB555ALIAS:
        case RGB565ALIAS:
        case RGB888ALIAS:
        case RGB555:
        case RGB565:
        case RGB888: {
            value = Enum(new_value);
            break;
        }
        default: {
            throw std::domain_error("IMGColorDepth: Inacceptable color depth value");
        }
    }
}

std::size_t IMGColorDepth::get_pixel_size() const
{
    switch (value) {
        case RGB555:
        case RGB555ALIAS:
        case RGB565:
        case RGB565ALIAS: {
            return 2;
        }
        case RGB888:
        case RGB888ALIAS: {
            return 3;
        }
        default: {
            return 0;
        }
    }
}

IMGColorDepth::IMGColorDepth(uint32_t value)
{
    set_value(value);
}

template<typename T, typename Iterator>
T from_raw_bytes(Iterator begin_it, Iterator end_it)
{
    static_assert(std::is_same<typename std::iterator_traits<Iterator>::value_type, unsigned char>::value);
    T ret_value;
    unsigned char *raw_ret_value = reinterpret_cast<unsigned char *>(&ret_value);
    auto it = begin_it;
    for (std::size_t i = 0; i < sizeof(T); i++) {
        if (it == end_it) {
            throw runtime_error("from_raw_bytes: input too short");
        }
        raw_ret_value[i] = *it;
        it++;
    }
    return ret_value;
}

void IMGFormat::memory_decompress(const std::vector<unsigned char> &input_memory)
{
    const int header_size = 40;
    //assert input_memory.size() >= 40

    std::array<unsigned char, 4> file_header = {'P', 'I', 'K', '\0'};
    for (std::size_t i = 0; i < file_header.size(); i++) {
        if (input_memory[i] != file_header[i]) {
            throw invalid_structure("IMGFormat: Incorrect file type");
        }
    }

    header.width = from_raw_bytes<uint32_t>(input_memory.begin() + file_header.size(), input_memory.end());
    header.height = from_raw_bytes<uint32_t>(input_memory.begin() + file_header.size() + sizeof(uint32_t), input_memory.end());
    uint32_t unsafe_color_depth = from_raw_bytes<uint32_t>(input_memory.begin() + file_header.size() + 2 * sizeof(uint32_t), input_memory.end());
    header.color_depth = unsafe_color_depth;
    header.image_size = from_raw_bytes<uint32_t>(input_memory.begin() + file_header.size() + 3 * sizeof(uint32_t), input_memory.end());
    uint32_t unsafe_compression = from_raw_bytes<uint32_t>(input_memory.begin() + file_header.size() + 5 * sizeof(uint32_t), input_memory.end());
    header.compression = unsafe_compression;
    header.alpha_size = from_raw_bytes<uint32_t>(input_memory.begin() + file_header.size() + 6 * sizeof(uint32_t), input_memory.end());
    header.x_position = from_raw_bytes<uint32_t>(input_memory.begin() + file_header.size() + 7 * sizeof(uint32_t), input_memory.end());
    header.y_position = from_raw_bytes<uint32_t>(input_memory.begin() + file_header.size() + 8 * sizeof(uint32_t), input_memory.end());

    if (input_memory.size() < header.image_size + header.alpha_size + header_size) {
        throw invalid_size("IMGFormat: Input data too short");
    }

    uncompressed_rgb_data.reserve(header.width * header.height * header.color_depth.get_pixel_size());
    if (header.alpha_size != 0) {
        uncompressed_alpha_data.reserve(header.width * header.height);
    } else {
        uncompressed_alpha_data.resize(0);
    }
    if (header.compression == IMGCompression::NONE_NONE) {
        uncompressed_rgb_data.assign(input_memory.begin() + header_size, input_memory.begin() + header_size + header.image_size);
        if (header.alpha_size != 0) {
            uncompressed_alpha_data.assign(input_memory.begin() + header_size + header.image_size, input_memory.begin() + header_size + header.image_size + header.alpha_size);
        }
    } else if (header.compression == IMGCompression::CLZW2_CLZW2) {
        std::vector<unsigned char> compressed_rgb_data(input_memory.begin() + header_size, input_memory.begin() + header_size + header.image_size);
        char *output_buffer;
        uint32_t output_size = from_raw_bytes<uint32_t>(compressed_rgb_data.begin(), compressed_rgb_data.end());
        output_buffer = piklib_CLZWCompression2_decompress(reinterpret_cast<char *>(compressed_rgb_data.data()), compressed_rgb_data.size());
        uncompressed_rgb_data.assign(output_buffer, output_buffer + output_size);
        piklib_CLZWCompression2_deallocate(output_buffer);
        if (header.alpha_size != 0) {
            std::vector<unsigned char> compressed_alpha_data(input_memory.begin() + header_size + header.image_size, input_memory.begin() + header_size + header.image_size + header.alpha_size);
            output_size = from_raw_bytes<uint32_t>(compressed_alpha_data.begin(), compressed_alpha_data.end());
            output_buffer = piklib_CLZWCompression2_decompress(reinterpret_cast<char *>(compressed_alpha_data.data()), compressed_alpha_data.size());
            uncompressed_alpha_data.assign(output_buffer, output_buffer + output_size);
            piklib_CLZWCompression2_deallocate(output_buffer);
        }
    } else if (header.compression == IMGCompression::JPEG_CLZW2) {
        throw not_implemented();
    }

}

void IMGFormat::memory_compress(std::vector<unsigned char> &output_memory)
{
    throw not_implemented();
}

const IMGHeader &IMGFormat::get_header() const
{
    return header;
}

const std::vector<unsigned char> &IMGFormat::get_pixmap_as_rgb555()
{
    if (header.color_depth == IMGColorDepth::RGB555 || header.color_depth == IMGColorDepth::RGB555ALIAS) {
        return uncompressed_rgb_data;
    }
    throw not_implemented();
}

const std::vector<unsigned char> &IMGFormat::get_pixmap_as_rgb565()
{
    if (header.color_depth == IMGColorDepth::RGB565 || header.color_depth == IMGColorDepth::RGB565ALIAS) {
        return uncompressed_rgb_data;
    }
    throw not_implemented();
}

const std::vector<unsigned char> &IMGFormat::get_pixmap_as_rgb888()
{
    if (header.color_depth == IMGColorDepth::RGB888 || header.color_depth == IMGColorDepth::RGB888ALIAS) {
        return uncompressed_rgb_data;
    }
    throw not_implemented();
}

const std::vector<unsigned char> &IMGFormat::get_pixmap_as_rgba8888()
{
    throw not_implemented();
}

const std::vector<unsigned char> &IMGFormat::get_original_rgb_pixmap()
{
    return uncompressed_rgb_data;
}

const std::vector<unsigned char> &IMGFormat::get_original_alpha_pixmap()
{
    return uncompressed_alpha_data;
}

IMGFormat IMGFormat::from_rgb555_pixmap(const std::vector<unsigned char> &pixmap, const IMGHeader &header)
{
    throw not_implemented();
}

IMGFormat IMGFormat::from_rgb565_pixmap(const std::vector<unsigned char> &pixmap, const IMGHeader &header)
{
    throw not_implemented();
}

IMGFormat IMGFormat::from_rgb888_pixmap(const std::vector<unsigned char> &pixmap, const IMGHeader &header)
{
    throw not_implemented();
}
IMGFormat IMGFormat::from_rgba8888_pixmap(const std::vector<unsigned char> &pixmap, const IMGHeader &header)
{
    throw not_implemented();
}

IMGFormat::IMGFormat(const std::vector<unsigned char> &memory)
{
    memory_decompress(memory);
}

IMGFormat::IMGFormat(const std::string &filename)
{
    #ifdef _GLIBCXX_FILESYSTEM
        std::filesystem::path file_path = filename;
        if (!std::filesystem::exists(file_path)) {
            throw std::runtime_error("IMGFormat: cannot find the specified file");
        }
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
    #else
        std::ifstream file_handle(filename, ios::binary);
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
    #endif // _GLIBCXX_FILESYSTEM
    memory_decompress(file_content);
}

ostream &operator<<(ostream &os, const IMGHeader &ih)
{
	os << "[IMG header]\n"
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
        "  Y position: " << ih.y_position << "px";
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

void determine_compression_format(IMGHeader &img_header, vector<unsigned char> &img_data_color)
{
	if (img_header.width * img_header.height * 2 == img_header.image_size) {
		img_header.compression = 0;
	} else {
		string test(reinterpret_cast<char *>(img_data_color.data()), 4);
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
