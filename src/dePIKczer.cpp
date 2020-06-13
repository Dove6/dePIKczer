// dePIKczer.cpp : Defines the entry point for the console application.
//

#include "../include/stdafx.h"

#define BUFF_SIZE 200

using namespace std;

class compression_failure : public runtime_error {
public:
	compression_failure(const string &what_arg)
		: runtime_error(what_arg)
	{}
	compression_failure(const char *what_arg)
		: runtime_error(what_arg)
	{}
	compression_failure()
		: runtime_error("Unknown compression error!\n")
	{}
	compression_failure(const compression_failure &e)
		: runtime_error(e.what())
	{}
};

class help_issued : public exception {
};

class invalid_size : public logic_error {
public:
	invalid_size(const string &what_arg)
		: logic_error(what_arg)
	{}
	invalid_size(const char *what_arg)
		: logic_error(what_arg)
	{}
	invalid_size()
		: logic_error("Unknown data size error!\n")
	{}
	invalid_size(const invalid_size &e)
		: logic_error(e.what())
	{}
};

class invalid_structure : public runtime_error {
public:
	invalid_structure(const string &what_arg)
		: runtime_error(what_arg)
	{}
	invalid_structure(const char *what_arg)
		: runtime_error(what_arg)
	{}
	invalid_structure()
		: runtime_error("Unknown file structure error!\n")
	{}
	invalid_structure(const invalid_structure &e)
		: runtime_error(e.what())
	{}
};

class io_failure : public runtime_error {
public:
	io_failure(const string &what_arg)
		: runtime_error(what_arg)
	{}
	io_failure(const char *what_arg)
		: runtime_error(what_arg)
	{}
	io_failure()
		: runtime_error("Unknown I/O error!\n")
	{}
	io_failure(const io_failure &e)
		: runtime_error(e.what())
	{}
};

class parsing_error : public runtime_error {
public:
	parsing_error(const string &what_arg)
		: runtime_error(what_arg)
	{}
	parsing_error(const char *what_arg)
		: runtime_error(what_arg)
	{}
	parsing_error()
		: runtime_error("Unknown console options parsing error!\n")
	{}
	parsing_error(const parsing_error &e)
		: runtime_error(e.what())
	{}
};

class path_error : public runtime_error {
public:
	path_error(const string &what_arg)
		: runtime_error(what_arg)
	{}
	path_error(const char *what_arg)
		: runtime_error(what_arg)
	{}
	path_error()
		: runtime_error("Unknown path-related error!\n")
	{}
	path_error(const parsing_error &e)
		: runtime_error(e.what())
	{}
};

class console_writer {
public:
	static console_writer &get_instance(bool cout_enabled = false)
	{
		static console_writer instance(cout_enabled);
		return instance;
	}
private:
	bool attached,
		 allocated;
	console_writer(bool cout_enabled)
		: attached(false), allocated(false)
	{
		attached = (AttachConsole(ATTACH_PARENT_PROCESS) != 0);
		if (!attached) {
			attached = (AllocConsole() != 0);
			allocated = attached;
		}
		if (attached) {
			if (cout_enabled) {
				HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);
				int system_output = _open_osfhandle(intptr_t(console_output), 0x4000);
				FILE *c_output_handle = _fdopen(system_output, "w");
				freopen_s(&c_output_handle, "CONOUT$", "w", stdout);
			}
			HANDLE console_error = GetStdHandle(STD_ERROR_HANDLE);
			int system_error = _open_osfhandle(intptr_t(console_error), 0x4000);
			FILE *c_error_handle = _fdopen(system_error, "w");
			freopen_s(&c_error_handle, "CONOUT$", "w", stderr);

			HANDLE console_input = GetStdHandle(STD_INPUT_HANDLE);
			int system_input = _open_osfhandle(intptr_t(console_input), 0x4000);
			FILE *c_input_handle = _fdopen(system_input, "r");
			freopen_s(&c_input_handle, "CONIN$", "r", stdin);

			cout << '\n';
		}
	}
	~console_writer()
	{
		if (attached) {
			for (int i = 0; i < 80 * 25; i++) {
				SendMessage(GetConsoleWindow(), WM_CHAR, '\b', 0);
			}
			if (allocated) {
				cin.get();
			}
			SendMessage(GetConsoleWindow(), WM_CHAR, '\r', 0);
			FreeConsole();
		}
	}
};

class tee_streambuf : public streambuf {
	streambuf *s1, *s2;
	virtual int overflow(int c)
	{
		if (c == EOF) {
			return !EOF;
		} else {
			const int ret1 = s1->sputc(c),
				      ret2 = s2->sputc(c);
			return (ret1 == EOF || ret2 == EOF ? EOF : c);
		}
	}
	virtual int sync()
	{
		const int ret1 = s1->pubsync(),
				  ret2 = s2->pubsync();
		return (ret1 == 0 && ret2 == 0 ? 0 : -1);
	}
public:
	tee_streambuf(streambuf *s1, streambuf *s2)
		: s1(s1), s2(s2)
	{}
};

class tee_ostream : public ostream {
	tee_streambuf s;
public:
	tee_ostream(ostream &o1, ostream &o2)
		: ostream(&s), s(o1.rdbuf(), o2.rdbuf())
	{}
};

struct IMGHEADER {
    		 char ihType[4];
    unsigned int  ihWidth;
    unsigned int  ihHeight;
    unsigned int  ihBitCount;
    unsigned int  ihSizeImage;
    		 int  ihNothing;
    		 int  ihCompression;
    unsigned int  ihSizeAlpha;
    		 int  ihPosX;
    		 int  ihPosY;

	IMGHEADER operator=(const IMGHEADER &ih)
	{
		copy(ih.ihType, ih.ihType + 4, ihType);
		ihWidth = ih.ihWidth;
		ihHeight = ih.ihHeight;
		ihBitCount = ih.ihBitCount;
		ihSizeImage = ih.ihSizeImage;
		ihNothing = ih.ihNothing;
		ihCompression = ih.ihCompression;
		ihSizeAlpha = ih.ihSizeAlpha;
		ihPosX = ih.ihPosX;
		ihPosY = ih.ihPosY;
		return ih;
	}

	IMGHEADER()
		:ihWidth(0), ihHeight(0), ihBitCount(0), ihSizeImage(0), ihNothing(0), ihCompression(0), ihSizeAlpha(0), ihPosX(0), ihPosY(0)
	{
		ihType[0] = 0;
		ihType[1] = 0;
		ihType[2] = 0;
		ihType[3] = 0;
	}
};

ostream &operator<<(ostream &os, const IMGHEADER &ih)
{
	os << "[IMG header]\n"
        "  type: " << string(ih.ihType, 4) << "\n"
        "  width: " << ih.ihWidth << "px\n"
        "  height: " << ih.ihHeight << "px\n"
        "  depth: " << ih.ihBitCount << "bpp\n"
        "  image size (color): " << ih.ihSizeImage << "B\n"
        "  padding: " << ih.ihNothing << "\n"
        "  compression: " << ih.ihCompression
            << (ih.ihCompression == 0 ? " (none)" :
                (ih.ihCompression == 2 ? " (CLZW2)" :
                (ih.ihCompression == 4 ? " (unspecified)" :
                (ih.ihCompression == 5 ? " (JPEG)" : "")))) << "\n"
        "  image size (alpha): " << ih.ihSizeAlpha << "B\n"
        "  X position: " << ih.ihPosX << "px\n"
        "  Y position: " << ih.ihPosY << "px\n";
	return os;
}

#include <pshpack2.h>
struct BITMAPHEADER {
    BITMAPFILEHEADER bf;
    BITMAPV5HEADER bV5;
};
#include <poppack.h>

enum output_format {
	BMP,
	JPG,
	PNG
};

struct cli_options {
	enum output_format format;
	bool decompress,
        custom_dir,
		add_game_name,
		verbose;
	string dir;
};

array<string, 6> names_to_append = {"RiSP", "RiU", "RiC", "RiWC", "RiKN", "RiKwA"};

array<vector<string>, 6> characteristic_names;

string get_winapi_error_msg(unsigned err_code)
{
	char *buffer = nullptr;
	unsigned buffer_size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0, err_code, 0, LPSTR(&buffer), 0, nullptr);
	string message(buffer, buffer_size);
	LocalFree(buffer);
	return message;
}

enum output_format parse_output_format(string input)
{
	for (unsigned i = 0; i < input.size(); i++) {
		if (input[i] < 91) {
			input[i] += 32;
		}
	}
	if (input == string("bmp")) {
		return BMP;
	} else if (input == string("jpg") || input == string("jpeg")) {
		return JPG;
	} else if (input == string("png")) {
		return PNG;
	} else {
		throw parsing_error("Unknown output format!\n");
	}
}

void parse_cli_options(const int argc, char **argv, cli_options &options, int &arg_iter)
{
	//defaults
	options.decompress = true;
	options.format = PNG;
	options.custom_dir = false;
	options.add_game_name = false;
	options.verbose = false;
	//parsing
	string arg;
	for (; arg_iter < argc; arg_iter++) {
		if (argv[arg_iter][0] == '/' || argv[arg_iter][0] == '-') {
			arg = argv[arg_iter];
			if ((arg.size() == 2 && arg[1] == 'd') ||
				(arg.find("decompress") != string::npos &&
				(arg.size() == 11 || (arg.size() == 12 && arg[1] == '-')))) {
				//-d, /d, -decompress, /decompress, --decompress
				options.decompress = true;
			} else if ((arg.size() == 2 && arg[1] == 'c') ||
						(arg.find("compress") != string::npos &&
						(arg.size() == 9 || (arg.size() == 10 && arg[1] == '-')))) {
				//-c, /c, -compress, /compress, --compress
				options.decompress = false;
				throw parsing_error("Not implemented!\n");
			} else if ((arg.size() == 2 && arg[1] == 'f') ||
						arg.substr(1, 10) == string("out-format") ||
						(arg.substr(2, 10) == string("out-format") && arg[1] == '-')) {
				//-f, /f, -out-format, /out-format, --out-format
				if (arg.size() == 2) {
					if (arg_iter + 1 < argc) {
						arg_iter++;
						options.format = parse_output_format(string(argv[arg_iter]));
					} else {
						throw parsing_error("Option argument missing!\n");
					}
				} else {
					if (arg.size() > 10U + (arg[1] == '-' ? 2 : 1) + 1) {
						options.format = parse_output_format(arg.substr(10 + (arg[1] == '-' ? 2 : 1) + 1)); //length + prefix ("--") + suffix ('=')
					} else {
						throw parsing_error("Option argument missing!\n");
					}
				}
			} else if ((arg.size() == 2 && arg[1] == 'g') ||
				(arg.find("add-game-name") != string::npos &&
				(arg.size() == 14 || (arg.size() == 15 && arg[1] == '-')))) {
				//-g, /g, -add-game-name, /add-game-name, --add-game-name
				options.add_game_name = true;
			} else if ((arg.size() == 2 && arg[1] == 'h') ||
						(arg.find("help") != string::npos &&
						(arg.size() == 5 || (arg.size() == 6 && arg[1] == '-')))) {
				//-h, /h, -help, /help, --help
				throw help_issued();
			} else if ((arg.size() == 2 && arg[1] == 'o') ||
						arg.substr(1, 7) == string("out-dir") ||
						(arg.substr(2, 7) == string("out-dir") && arg[1] == '-')) {
				//-o, /o, -out-dir, /out-dir, --out-dir
				options.custom_dir = true;
				if (arg.size() == 2) {
					if (arg_iter + 1 < argc) {
						arg_iter++;
						options.dir = string(argv[arg_iter]);
					} else {
						throw parsing_error("Option argument missing!\n");
					}
				} else {
					if (arg.size() > 7U + (arg[1] == '-' ? 2 : 1) + 1) {
						options.dir = arg.substr(7 + (arg[1] == '-' ? 2 : 1) + 1); //length + prefix ("--") + suffix ('=')
					} else {
						throw parsing_error("Option argument missing!\n");
					}
				}
			} else if ((arg.size() == 2 && arg[1] == 'v') ||
				(arg.find("verbose") != string::npos &&
				(arg.size() == 8 || (arg.size() == 9 && arg[1] == '-')))) {
				//-v, /v, -verbose, /verbose, --verbose
				options.verbose = true;
			} else {
				throw parsing_error("Unknown option!\n");
			}
		} else {
			return;
		}
	}
}

void push_back_characteristic_names()
{
	characteristic_names[0].push_back("pirat");
	characteristic_names[0].push_back("skarb");
	characteristic_names[0].push_back("mainmenu");
	characteristic_names[0].push_back("risp");
	characteristic_names[1].push_back("ufo");
	characteristic_names[1].push_back("riu");
	characteristic_names[2].push_back("czaro");
	characteristic_names[2].push_back("ric");
	characteristic_names[3].push_back("wehiku");
	characteristic_names[3].push_back("czasu");
	characteristic_names[3].push_back("riwc");
	characteristic_names[4].push_back("nemo");
	characteristic_names[4].push_back("kapitan");
	characteristic_names[4].push_back("rikn");
	characteristic_names[5].push_back("akcji");
	characteristic_names[5].push_back("rikwa");
}

string compose_out_filename(char **argv, const int starting_argument, const int arg_iter, const cli_options &options)
{
	string result;
	string exe_path;
	if (starting_argument == 1) {
		exe_path = argv[0];
	}
	string element = argv[arg_iter];
	//appending directory
	if (options.custom_dir) {
		if (!CreateDirectoryA(options.dir.c_str(), 0)) {
			unsigned err = GetLastError();
			if (err == ERROR_PATH_NOT_FOUND) {
				throw path_error("Incorrect output directory path!");
			} else if (err != ERROR_ALREADY_EXISTS) {
				throw path_error(get_winapi_error_msg(err));
			}
		}
		result = options.dir;
	} else {
		if (element.find_last_of('\\') != string::npos) {
			result.append(element.substr(0, element.find_last_of('\\')));
		}
	}
	if (result.size() > 0) {
		if (result.back() != '\\') {
			result.append("\\");
		}
	}
	//input filename part
	if (element.find_last_of('\\') != string::npos) {
		result.append(element.substr(element.find_last_of('\\') + 1));
	} else {
		result.append(element);
	}
	if (result[result.size() - 4] == '.') {
		result.erase(result.size() - 4);
	}
	//game name part
	for (unsigned i = 0; i < exe_path.size(); i++) {
		if (exe_path[i] > 64 && exe_path[i] < 91) {
			exe_path[i] += 32;
		}
	}
	for (unsigned i = 0; i < element.size(); i++) {
		if (element[i] > 64 && element[i] < 91) {
			element[i] += 32;
		}
	}
	if (options.add_game_name) {
		for (int i = 0; i < 6; i++) {
			for (unsigned j = 0; j < characteristic_names[i].size(); j++) {
				if (exe_path.find(characteristic_names[i][j]) != string::npos || element.find(characteristic_names[i][j]) != string::npos) {
					result.append("_");
					result.append(names_to_append[i]);
					i = 6;
					break;
				}
			}
		}
	}
	//number part
	string core_filename = result;
	string extension;
	switch (options.format) {
		case BMP: {
			extension = ".bmp";
			break;
		}
		case JPG: {
			extension = ".jpg";
			break;
		}
		case PNG: {
			extension = ".png";
			break;
		}
	}
	result.append(extension);
	ifstream existance_test(result, ios::in);
	unsigned i;
	for (i = 2; existance_test.good() && i <= 1000; i++) {
		existance_test.close();
		result = core_filename + string("_") + to_string((unsigned long long)(i)) + extension;
		existance_test.open(result, ios::in);
	}
	if (i == 1000) {
		throw path_error("Couldn't compose output filename!");
	}
	return result;
}

IMGHEADER read_img_header(ifstream &img_file)
{
    iostream::pos_type init_pos;
	init_pos = img_file.tellg();
	img_file.seekg(0, ios::beg);

    IMGHEADER img_header;
	try {
		img_file.read((char *)(&img_header), sizeof(IMGHEADER));
	} catch (const ifstream::failure &) {
		throw io_failure("Error reading image header!");
	}

	cout << "Read IMG header!\n";

	img_file.seekg(init_pos);
    return img_header;
}

void check_img_header(const IMGHEADER &img_header)
{
	if (img_header.ihType != string("PIK")) {
		throw invalid_structure("Incorrect header identifier!");
	}
	if (img_header.ihBitCount != 15 && img_header.ihBitCount != 16) {
		throw invalid_structure("Unknown color format!");
	}
	if (img_header.ihCompression != 0 && img_header.ihCompression != 2 && img_header.ihCompression != 4 && img_header.ihCompression != 5) {
		throw invalid_structure("Unknown compression!");
	}
	if (img_header.ihNothing != 0) {
		throw invalid_structure("Unexpected padding value!");
	}
	cout << "The header is correct!\n";
}

void read_img_data(ifstream &img_file, IMGHEADER &img_header, vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	iostream::pos_type init_pos;
	init_pos = img_file.tellg();
	img_file.seekg(40, ios::beg);

	char *buffer;
	buffer = new char[img_header.ihSizeImage];
	try {
		img_file.read(buffer, img_header.ihSizeImage);
	} catch (ifstream::failure &) {
        img_header.ihSizeAlpha = 0;
		for (streamsize i = img_file.gcount(); i < int(img_header.ihSizeImage); i++) {
			buffer[i] = 0;
		}
		img_data_color.assign(buffer, buffer + img_header.ihSizeImage);
		delete[] buffer;
		throw io_failure("Error reading color image data! (padded with zeroes)");
    }
	img_data_color.assign(buffer, buffer + img_header.ihSizeImage);
	delete[] buffer;

	if (img_header.ihSizeAlpha != 0) {
		buffer = new char[img_header.ihSizeAlpha];
		try {
			img_file.read(buffer, img_header.ihSizeAlpha);
			img_data_alpha.assign(buffer, buffer + img_header.ihSizeAlpha);
		} catch (ifstream::failure &) {
			img_header.ihSizeAlpha = 0;
			delete[] buffer;
			throw io_failure("Error reading alpha image data! (skipped)");
		}
		delete[] buffer;
	}

    img_file.seekg(init_pos);
}

void determine_compression_format(IMGHEADER &img_header, vector<char> &img_data_color)
{
	if (img_header.ihWidth * img_header.ihHeight * 2 == img_header.ihSizeImage) {
		img_header.ihCompression = 0;
	} else {
		string test(img_data_color.data(), 4);
		if (test == string("\xFF\xD8\xFF\xE0")) {
			img_header.ihCompression = 5;
		} else {
			test.assign(img_data_color.end() - 3, img_data_color.end());
			if (test == string("\x11\x00\x00")) {
				img_header.ihCompression = 2;
			} else {
				throw invalid_structure("Unknown compression!");
			}
		}
	}
}

struct BITMAPHEADER *prepare_bmp_header(const IMGHEADER &img_header, const vector<char> &bmp_data)
{
    BITMAPHEADER *bmp_header = new BITMAPHEADER;

    bmp_header->bf.bfType = 0x4D42;
    bmp_header->bf.bfReserved1 = 0;
    bmp_header->bf.bfReserved2 = 0;
    bmp_header->bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV5HEADER);

    bmp_header->bV5.bV5Size = sizeof(BITMAPV5HEADER);
    bmp_header->bV5.bV5Width = img_header.ihWidth;
    bmp_header->bV5.bV5Height = img_header.ihHeight * -1; //-1 for upside-down image
    bmp_header->bV5.bV5Planes = 1;
    bmp_header->bV5.bV5BitCount = 16;
	bmp_header->bV5.bV5Compression = BI_BITFIELDS;
    bmp_header->bV5.bV5SizeImage = bmp_data.size();
    bmp_header->bV5.bV5XPelsPerMeter = 2835;
    bmp_header->bV5.bV5YPelsPerMeter = 2835;
    bmp_header->bV5.bV5ClrUsed = 0;
    bmp_header->bV5.bV5ClrImportant = 0;
	switch (img_header.ihBitCount) {
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

	if (img_header.ihSizeAlpha > 0) {
		bmp_header->bV5.bV5BitCount = 32;
		bmp_header->bV5.bV5RedMask =   0x000000FF;
		bmp_header->bV5.bV5GreenMask = 0x0000FF00;
		bmp_header->bV5.bV5BlueMask =  0x00FF0000;
		bmp_header->bV5.bV5AlphaMask = 0xFF000000;
	} else if (img_header.ihCompression == 5) {
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

void compress_jpg(vector<char> &img_data_color, const IMGHEADER &img_header)
{
	tjhandle compressor = tjInitCompress();
	if (compressor != nullptr) {
		unsigned long buffer_size = tjBufSize(img_header.ihWidth, img_header.ihHeight, TJSAMP_444);
		char *buffer = new char[buffer_size];
		if (!tjCompress2(compressor, (const unsigned char *)(img_data_color.data()), img_header.ihWidth, 0, img_header.ihHeight,
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

void decompress_jpg(vector<char> &img_data_color, const IMGHEADER &img_header, enum output_format format)
{
	tjhandle decompressor = tjInitDecompress();
	if (decompressor != nullptr) {
		unsigned buffer_size = img_header.ihWidth * img_header.ihHeight * (img_header.ihSizeAlpha > 0 ? 4 : 3);
		char *buffer = new char[buffer_size];
		int pixel_format;
		if (img_header.ihSizeAlpha > 0) {
			pixel_format = TJPF_RGBA;
		} else {
			if (format == BMP) {
				pixel_format = TJPF_BGR;
			} else {
				pixel_format = TJPF_RGB;
			}
		}
		if (!tjDecompress2(decompressor, (const unsigned char *)(img_data_color.data()), img_data_color.size(), (unsigned char *)(buffer),
						  img_header.ihWidth, img_header.ihWidth * (img_header.ihSizeAlpha > 0 ? 4 : 3), img_header.ihHeight,
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

vector<char> prepare_bmp_data(IMGHEADER &img_header, vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	switch (img_header.ihCompression) {
		case 2: {
			decompress_img(img_data_color, img_data_alpha);
			if (img_data_color.size() != img_header.ihWidth * img_header.ihHeight * 2) {
				throw invalid_size("Incorrect color image data size!");
			}
			break;
		}
		case 5: {
			decompress_jpg(img_data_color, img_header, BMP);
			vector<char> placeholder(0);
			decompress_img(placeholder, img_data_alpha);
			break;
		}
	}
	vector<char> aligned_bmp_data(0);
	char *buffer = (char *)(img_data_color.data());
	unsigned buffer_size = img_data_color.size();
	if (img_header.ihSizeAlpha == 0) {
		if (img_header.ihCompression == 5) {
			if (img_data_color.size() != img_header.ihWidth * img_header.ihHeight * 3) {
				throw invalid_size("Incorrect color image data size!");
			}
		}
		if (img_header.ihBitCount == 15) { //for 16bpp go straight to padding check
			buffer_size = img_data_color.size();
			buffer = new char[buffer_size];
			copy(img_data_color.begin(), img_data_color.end(), buffer);
			#ifdef _DEBUG
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
			#ifdef _DEBUG
				clog << "[log] Converted image color format!\n";
			#endif
			//go to padding check
		}
	} else {
		if (img_header.ihCompression == 5) {
			if (img_data_color.size() != img_header.ihWidth * img_header.ihHeight * 4) {
				throw invalid_size("Incorrect color image data size!");
			}
		}
		unsigned pixel_count = img_header.ihWidth * img_header.ihHeight;
		if (img_data_color.size() / 2 == img_data_alpha.size()) {
			buffer_size = pixel_count * 4;
			buffer = new char[buffer_size];
			char pixel[4];
			if (img_header.ihBitCount == 16) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
					pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			} else if (img_header.ihBitCount == 15) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
					pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			}
		} else if (img_data_color.size() / 4 == img_data_alpha.size() && img_header.ihCompression == 5) {
			for (unsigned i = 0; i < pixel_count; i++) {
				//after JPEG decompression of IMG with alpha img_data_color has "holes" every 4 bytes
				//and buffer points to img_data_color.data()
				//so basically we are filling these holes here
				buffer[i * 4 + 3] = img_data_alpha[i];
			}
		} else {
			img_header.ihSizeAlpha = 0;
			throw invalid_size("Unknown alpha data format!\n");
		}
	}

	if (buffer_size / img_header.ihHeight % 4 > 0) {
		unsigned row_length = buffer_size / img_header.ihHeight;
		unsigned padding_length = 4 - row_length % 4;
		unsigned padded_row_length = row_length + padding_length;
		unsigned padded_buffer_size = padded_row_length * img_header.ihHeight;
		char padding[4] = {0, 0, 0, 0};
		char *padded_buffer = new char[padded_buffer_size];
		for (unsigned i = 0; i < img_header.ihHeight; i++) {
			copy(buffer + i * row_length, buffer + (i + 1) * row_length, padded_buffer + i * padded_row_length);
			copy(padding, padding + padding_length, padded_buffer + i * padded_row_length + row_length);
		}
		#ifdef _DEBUG
			clog << "[log] Converted image data!\n";
		#endif
		aligned_bmp_data.reserve(padded_buffer_size);
		aligned_bmp_data.assign(padded_buffer, padded_buffer + padded_buffer_size);
		delete[] padded_buffer;
	} else {
		#ifdef _DEBUG
			clog << "[log] Converted image data!\n";
		#endif
		aligned_bmp_data.reserve(buffer_size);
		aligned_bmp_data.assign(buffer, buffer + buffer_size);
	}

	if (buffer != img_data_color.data()) {
		delete[] buffer;
	}
	return aligned_bmp_data;
}

vector<char> prepare_jpg_data(const IMGHEADER &img_header, vector<char> &img_data_color)
{
	if (img_header.ihCompression != 5) {
		if (img_header.ihCompression == 2) {
            vector<char> placeholder(0);
			decompress_img(img_data_color, placeholder);
		}
		if (img_data_color.size() != img_header.ihWidth * img_header.ihHeight * 2) {
			throw invalid_size("Incorrect color image data size!");
		}
		unsigned pixel_count = img_header.ihWidth * img_header.ihHeight;
		unsigned buffer_size = pixel_count * 3;
		char *buffer = new char[buffer_size];
		char pixel[3];
		if (img_header.ihBitCount == 15) {
			for (unsigned i = 0; i < pixel_count; i++) {
				pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
				pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
				pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
				copy(pixel, pixel + 3, buffer + i * 3);
			}
		} else if (img_header.ihBitCount == 16) {
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
	} else {
		return img_data_color;
	}
}

vector<char> prepare_png_data(IMGHEADER &img_header, vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	vector<char> png_data(0);
	if (img_header.ihCompression != 5) {
		if (img_header.ihCompression == 2) {
			decompress_img(img_data_color, img_data_alpha);
		}
		if (img_data_color.size() != img_header.ihWidth * img_header.ihHeight * 2) {
			throw invalid_size("Incorrect color image data size!");
		}
		unsigned pixel_count = img_header.ihWidth * img_header.ihHeight;
		unsigned buffer_size = pixel_count * (img_header.ihSizeAlpha > 0 ? 4 : 3);
		char *buffer = new char[buffer_size];
		if (img_header.ihSizeAlpha == 0) {
			char pixel[3];
			if (img_header.ihBitCount == 15) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
					pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					copy(pixel, pixel + 3, buffer + i * 3);
				}
			} else if (img_header.ihBitCount == 16) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
					pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					copy(pixel, pixel + 3, buffer + i * 3);
				}
			}
		} else if (img_data_color.size() / 2 == img_data_alpha.size()) {
			char pixel[4];
			if (img_header.ihBitCount == 15) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
					pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			} else if (img_header.ihBitCount == 16) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
					pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			}
		} else {
			img_header.ihSizeAlpha = 0;
			throw runtime_error("Unknown alpha data format!\n");
		}
		img_data_color.assign(buffer, buffer + buffer_size);
		delete[] buffer;
	} else {
		decompress_jpg(img_data_color, img_header, PNG);
		vector<char> placeholder(0);
		decompress_img(placeholder, img_data_alpha);
		if (img_header.ihSizeAlpha == 0) {
			if (img_data_color.size() != img_header.ihWidth * img_header.ihHeight * 3) {
				throw invalid_size("Incorrect color image data size!");
			}
		} else {
			if (img_data_color.size() != img_header.ihWidth * img_header.ihHeight * 4) {
				throw invalid_size("Incorrect color image data size!");
			}
		}
		if (img_data_color.size() / 4 == img_data_alpha.size()) {
			unsigned pixel_count = img_header.ihWidth * img_header.ihHeight;
			for (unsigned i = 0; i < pixel_count; i++) {
				img_data_color[i * 4 + 3] = img_data_alpha[i];
			}
		} else if (img_header.ihSizeAlpha > 0) {
			img_header.ihSizeAlpha = 0;
			throw runtime_error("Unknown alpha data format!\n");
		}
	}
	size_t buffer_size;
	char *buffer;
	buffer = (char *)tdefl_write_image_to_png_file_in_memory_ex(img_data_color.data(), img_header.ihWidth, img_header.ihHeight,
        (img_header.ihSizeAlpha > 0 ? 4 : 3), &buffer_size, MZ_BEST_COMPRESSION, false);
	if (buffer != nullptr && buffer_size != 0) {
		png_data.assign(buffer, buffer + buffer_size);
		mz_free(buffer);
	} else {
		throw compression_failure("Error converting image data to PNG format!\n");
	}
	return png_data;
}

void write_bmp(ofstream &bmp_file, const IMGHEADER &img_header, const vector<char> &bmp_data)
{
	BITMAPHEADER *bmp_header = prepare_bmp_header(img_header, bmp_data);
	bmp_file.write((char *)(bmp_header), sizeof(BITMAPHEADER));
	bmp_file.write(bmp_data.data(), bmp_data.size());
}

void write_converted(ofstream &out_file, const vector<char> &out_data)
{
	out_file.write(out_data.data(), out_data.size());
}

void print_help()
{
	console_writer::get_instance(true);
	cout << "Usage:\n"
			"  dePIKczer [options] filename1[, filename2, ...]\n"
			"Available options (have to be prefixed with / or -):\n"
			"  d,                  decompress IMG file (default)\n"
			"  decompress\n"
			"  c,                  convert BMP, JPG or PNG file to IMG\n"
			"  compress              (not implemented yet)\n"
			"  f [format],         specify output file format\n"
			"  out-format=[format]   (for decompression only)\n"
			"                        available formats:\n"
			"                          bmp,\n"
			"                          jpg, jpeg,\n"
			"                          png (default)\n"
			"  g                   enable adding game suffix to output filenames\n"
			"  add-game-name         (experimental)\n"
			"  h,                  show this help message\n"
			"  help\n"
			"  o [sciezka],        use given directory for storing output files\n"
			"  out-dir=[sciezka]     (input files directory is used by default)\n"
			"  v,                  print additional debug data (forces console window\n"
			"  verbose               to be shown)\n";
}

void log_error(const char *description)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n\n";
		log_file.close();
	} else {
		cerr << description << '\n';
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

void log_error(const char *description, const char *error_string)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n  " << error_string << "\n\n";
		log_file.close();
	} else {
		cerr << description << "\n  " << error_string << '\n';
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

void log_error(const char *description, const char *error_string, const char *subject)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n  " << "for " << subject << "  \n" << error_string << "\n\n";
		log_file.close();
	} else {
		cerr << description << "\n  " << "for " << subject << "  \n" << error_string << '\n';
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

void log_error(const char *description, const char *error_string, const char *subject, const IMGHEADER &img_header)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n  for " << subject << "\n  " << error_string << '\n' << img_header << '\n';
		log_file.close();
	} else {
		cerr << description << "\n  for " << subject << "\n  " << error_string << '\n' << img_header;
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

void log_error(const char *description, const char *error_string, const char *subject, const char *additional_info)
{
	console_writer::get_instance();
	string log_path = getenv("appdata");
	log_path.append("\\dePIKczer");
	CreateDirectoryA(log_path.c_str(), 0);
	ofstream log_file(log_path + string("\\error.log"), ios::out | ios::app);
	if (log_file.good()) {
		tee_ostream errout(cerr, log_file);
		errout << description << "\n  " << "for " << subject << "\n  " << error_string << "\n  (" << additional_info << ")\n\n";
		log_file.close();
	} else {
		cerr << description << "\n  " << "for " << subject << "\n  " << error_string << "\n  (" << additional_info << ")\n";
		cerr << "[Couldn't write to the log file]\n\n";
	}
}

int main(int argc, char **argv)
{
	push_back_characteristic_names();
	int starting_argument = 1;
	if (argc > 0) {
		string img_test = argv[0];
		img_test.erase(0, img_test.size() - 3);
		for (int i = 0; i < 3; i++) {
			if (img_test[i] > 64 && img_test[i] < 91) {
				img_test[i]++;
			}
		}
		if (argv[0][0] == '-' || argv[0][0] == '/' || img_test == string("img")) {
			starting_argument--;
		}
	}
    if (argc > starting_argument) {
		int arg_iter = starting_argument;
		cli_options options;
		try {
			parse_cli_options(argc, argv, options, arg_iter);
			if (options.verbose) {
				console_writer::get_instance(true);
			}
			for (; arg_iter < argc; arg_iter++) {
				ifstream in_file(argv[arg_iter], ios::in | ios::binary);
				if (in_file.good()) {
					in_file.exceptions(ifstream::failbit);
					cout << "Processing " << argv[arg_iter] << "...\n";
					IMGHEADER img_header;
					try {
						img_header = read_img_header(in_file);
						check_img_header(img_header);
						cout << img_header;
						IMGHEADER img_header_mutable(img_header);

						vector<char> img_data_color, img_data_alpha;
						try {
							read_img_data(in_file, img_header_mutable, img_data_color, img_data_alpha);
						} catch (const io_failure &e) {
							log_error("I/O failure!", e.what(), argv[arg_iter], img_header);
						}
						cout << "Read the image data!\n";
						cout << "Size (in bytes): " << img_data_color.size() + img_data_alpha.size();
						if (img_data_alpha.size() > 0) {
							cout << " (including alpha: " << img_data_alpha.size() << ')';
						}
						cout << '\n';
						if (img_header.ihCompression == 4) { //unspecified/experimental
							determine_compression_format(img_header_mutable, img_data_color);
							cout << "Unsepcified compression format determined: " << img_header_mutable.ihCompression << '\n';
						}
						string out_filename;
						out_filename = compose_out_filename(argv, starting_argument, arg_iter, options);
						ofstream out_file(out_filename, ios::out | ios::binary);
						if (out_file.good()) {
							vector<char> converted_data;
							switch (options.format) {
								case BMP: {
									converted_data = prepare_bmp_data(img_header_mutable, img_data_color, img_data_alpha);
									cout << "Successfully converted to BMP!\n";
									cout << "New size (in bytes): " << converted_data.size() << '\n';
									break;
								}
								case JPG: {
									converted_data = prepare_jpg_data(img_header_mutable, img_data_color);
									cout << "Successfully converted to JPG!\n";
									cout << "New size (in bytes): " << converted_data.size() << '\n';
									break;
								}
								case PNG: {
									converted_data = prepare_png_data(img_header_mutable, img_data_color, img_data_alpha);
									cout << "Successfully converted to PNG!\n";
									cout << "New size (in bytes): " << converted_data.size() << '\n';
									break;
								}
							}
							switch (options.format) {
								case BMP: {
									write_bmp(out_file, img_header, converted_data);
									break;
								}
								default: {
									write_converted(out_file, converted_data);
									break;
								}
							}
							cout << "Saved to " << out_filename << "!\n";
							out_file.close();
						} else {
							log_error("Error opening output file!", strerror(errno), argv[arg_iter], out_filename.c_str());
						}
					} catch (const compression_failure &e) {
						log_error("Compression failure!", e.what(), argv[arg_iter], img_header);
					} catch (const invalid_size &e) {
						log_error("Invalid size!", e.what(), argv[arg_iter], img_header);
					} catch (const invalid_structure &e) {
						log_error("Invalid IMG header structure!", e.what(), argv[arg_iter], img_header);
					} catch (const path_error &e) {
						log_error("Path-related error!", e.what(), argv[arg_iter], img_header);
					} catch (const exception &e) {
						log_error("Unexpected exception inside the main loop!", e.what(), argv[arg_iter], img_header);
					}
				} else {
					log_error("Error opening input file!", strerror(errno), argv[arg_iter]);
				}
				in_file.close();
				cout << '\n';
			}
		} catch (const parsing_error &e) {
			log_error("Console options parsing error!", e.what(), argv[arg_iter]);
		} catch (const help_issued &) {
			print_help();
		} catch (const exception &e) {
			log_error("Unexpected exception during the initialization!", e.what());
		}
    } else {
		log_error("Error: No input files specified!\n");
        print_help();
    }
    return 0;
}
