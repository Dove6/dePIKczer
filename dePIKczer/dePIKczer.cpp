// dePIKczer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BUFF_SIZE 200

using namespace std;

class CLZWCompression {
	unsigned long __compress(unsigned char *, unsigned char *, unsigned long);
	void __decompress(char *, char *, long);
public:
	int *pre_spacing[4];
	int input_size;
	char *input_ptr;
	int *post_spacing[2];

	char *compress(int &);
	char *decompress(int);
	
	CLZWCompression(char *, int);
};

class CLZWCompression2 {

public:
	int *pre_spacing[4];
	int input_size;
	char *input_ptr;
	int *post_spacing[2];

	char *compress(int &);
	char *decompress(void);
	
	CLZWCompression2(char *, int);
};

class help_issued : public exception {
};

class console_writer {
public:
	static console_writer &get_instance(int argc, bool forced_silence = false)
	{
		static console_writer instance(argc, forced_silence);
		return instance;
	}
private:
	bool attached,
		 allocated;
	console_writer(int argc, bool forced_silence)
	: attached(false), allocated(false)
	{
		if (!forced_silence) {
			attached = AttachConsole(ATTACH_PARENT_PROCESS);
			if (!attached && argc <= 1) {
				attached = AllocConsole();
				allocated = attached;
			}
			if (attached) {
				HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);
				int system_output = _open_osfhandle(intptr_t(console_output), 0x4000);
				FILE *c_output_handle = _fdopen(system_output, "w");
				freopen_s(&c_output_handle, "CONOUT$", "w", stdout);
			
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
	}
	console_writer(const console_writer &c) {}
	console_writer &operator=(const console_writer &c) {}
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

struct IMGHEADER {
    char ihType[4];
    int  ihWidth;
    int  ihHeight;
    int  ihBitCount;
    int  ihSizeImage;
    int  something;
    int  ihCompression;
    int  ihSizeAlpha;
    int  ihPosX;
    int  ihPosY;
};

#include <pshpack2.h>
struct BITMAPHEADER {
    BITMAPFILEHEADER bf;
    BITMAPV5HEADER   bV5;
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
		 parsed_without_error,
		 silence;
	string dir;
};

array<string, 6> names_to_append = {"RiSP", "RiU", "RiC", "RiWC", "RiKN", "RiKwA"};

array<vector<string>, 6> characteristic_names;

/*int CLZW2_wrapper(int argc, char **argv)
{
	if (argc > 1) {
		bool decode = false;
		if (argv[1][0] == '/' && argv[1][1] == 'd') {
			decode = true;
		}
		int i = (decode ? 2 : 1);
		for (; i < argc; i++) {
			FILE *in_file = fopen(argv[i], "rb");
			if (in_file != NULL) {
				fseek(in_file, 0, SEEK_END);
				int in_size = ftell(in_file);
				rewind(in_file);

				char *input = (char *)malloc(in_size * 10 * sizeof(char));
				int read = fread(input, 1, in_size, in_file);
				fclose(in_file);
				std::cout << "Wczytano plik " << argv[i] << " o dlugosci " << read << '\n';

				CLZWCompression2 lzw(input, read);

				char *alt_filename = (char *)malloc((strlen(argv[i]) + 5) * sizeof(char));
				strcpy(alt_filename, argv[i]);
				strcat(alt_filename, (decode ? ".dek" : ".kod"));

				FILE *out_file = fopen(alt_filename, "wb");
				if (out_file != NULL) {
					if (decode) {
						char *output = lzw.decompress();
						int out_size = reinterpret_cast<int *>(input)[0];
						fwrite(output, 1, out_size, out_file);
						std::cout << "Zapisano zdekompresowany plik " << alt_filename << " o dlugosci " << out_size << '\n';
						std::cout << "Stosunek kompresji: " << (float)read / out_size * 100 << "%\n";
					} else {
						int out_size = 0;
						char *output = lzw.compress(out_size);
						out_size += 8;
						fwrite(output, 1, out_size, out_file);
						std::cout << "Zapisano skompresowany plik " << alt_filename << " o dlugosci " << out_size << '\n';
						std::cout << "Stosunek kompresji: " << (float)out_size / read * 100 << "%\n";
					}
					fclose(out_file);
				} else {
					std::cout << "Blad otwierania pliku wyjsciowego " << alt_filename << '\n';
				}
				free(alt_filename);
				free(input);
			} else {
				std::cout << "Blad otwierania pliku wejsciowego " << argv[i] << '\n';
			}
		}
	} else {
		std::cout << "Zbyt malo argumentow\n";
	}

	return 0;
}*/

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
		throw invalid_argument("Nieznany format wyjsciowy!\n");
	}
}

void parse_cli_options(const int argc, char **argv, cli_options &options, int &arg_iter)
{
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
				throw invalid_argument("Funkcjonalnosc niezaimplementowana!\n");
			} else if ((arg.size() == 2 && arg[1] == 'f') ||
						arg.substr(1, 10) == string("out-format") ||
						(arg.substr(2, 10) == string("out-format") && arg[1] == '-')) {
				//-f, /f, -out-format, /out-format, --out-format
				if (arg.size() == 2) {
					if (arg_iter + 1 < argc) {
						arg_iter++;
						options.format = parse_output_format(string(argv[arg_iter]));
					} else {
						throw out_of_range("Brak drugiej czesci opcji!\n");
					}
				} else {
					if (arg.size() > 10 + (arg[1] == '-' ? 2 : 1) + 1) {
						options.format = parse_output_format(arg.substr(10 + (arg[1] == '-' ? 2 : 1) + 1)); //length + prefix ("--") + suffix ('=')
					} else {
						throw out_of_range("Brak drugiej czesci opcji!\n");
					}
				}
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
						throw out_of_range("Brak drugiej czesci opcji!\n");
					}
				} else {
					if (arg.size() > 7 + (arg[1] == '-' ? 2 : 1) + 1) {
						options.dir = arg.substr(7 + (arg[1] == '-' ? 2 : 1) + 1); //length + prefix ("--") + suffix ('=')
					} else {
						throw out_of_range("Brak drugiej czesci opcji!\n");
					}
				}
			} else if ((arg.size() == 2 && arg[1] == 's') ||
				(arg.find("silent") != string::npos &&
				(arg.size() == 7 || (arg.size() == 8 && arg[1] == '-')))) {
				//-s, /s, -silent, /silent, --silent
				options.silence = true;
			} else {
				throw invalid_argument("Nierozpoznana opcja!\n");
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

void compose_out_filename(string &out_filename, char **argv, const int arg_iter, enum output_format format)
{
	string result;
	string exe_path = argv[0];
	string element = argv[arg_iter];
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
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < characteristic_names[i].size(); j++) {
			if (exe_path.find(characteristic_names[i][j]) != string::npos || element.find(characteristic_names[i][j]) != string::npos) {
				result.append(names_to_append[i]);
				i = 6;
				break;
			}
		}
	}
	//number part
	out_filename.append(result);
	string core_filename = out_filename;
	string extension;
	switch (format) {
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
	out_filename.append(extension);
	ifstream existance_test(out_filename, ios::in);
	for (unsigned i = 2; existance_test.good(); i++) {
		existance_test.close();
		out_filename = core_filename + string("_") + to_string(_ULonglong(i)) + extension;
		existance_test.open(out_filename, ios::in);
	}
}

IMGHEADER *read_img_header(ifstream &img_file)
{
    iostream::pos_type init_pos;
	init_pos = img_file.tellg();
	img_file.seekg(0, ios::beg);

    IMGHEADER *img_header = new IMGHEADER;
	if (img_file.read((char *)(img_header), sizeof(IMGHEADER))) {
		if (img_header->ihType == string("PIK")) {
			cout << "Poprawny typ pliku\n";
			cout << "Format: " << img_header->ihCompression;
			switch (img_header->ihCompression) {
				case 0: {
					cout << " (bez kompresji)\n";
					break;
				}
				case 2: {
					cout << " (CLZW2)\n";
					break;
				}
				case 4: {
					cout << " (?)\n";
					break;
				}
				case 5: {
					cout << " (JPG)\n";
					break;
				}
				default: {
					cout << " (format niestandardowy)\n";
					break;
				}
			}
			cout << "BPP: " << img_header->ihBitCount << '\n';
		} else {
			cerr << "Nieprawidlowy typ pliku!\n";
			delete img_header;
			img_header = nullptr;
		}
	} else {
		cerr << "Blad wczytywania pliku!\n";
		delete img_header;
		img_header = nullptr;
	}

	img_file.seekg(init_pos);
    return img_header;
}

void read_img_data(ifstream &img_file, IMGHEADER *img_header, vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	iostream::pos_type init_pos;
	init_pos = img_file.tellg();
	img_file.seekg(40, ios::beg);
	
	char *buffer;
	buffer = new char[img_header->ihSizeImage];
	img_file.read(buffer, img_header->ihSizeImage);
	img_data_color.assign(buffer, buffer + img_header->ihSizeImage);
	delete[] buffer;

	if (img_header->ihSizeAlpha != 0) {
		buffer = new char[img_header->ihSizeAlpha];
		img_file.read(buffer, img_header->ihSizeAlpha);
		img_data_alpha.assign(buffer, buffer + img_header->ihSizeAlpha);
		delete[] buffer;
	}

    img_file.seekg(init_pos);
}

struct BITMAPHEADER *prepare_bmp_header(IMGHEADER *img_header, const vector<char> &bmp_data)
{
    BITMAPHEADER *bmp_header = new BITMAPHEADER;

    bmp_header->bf.bfType = 0x4D42;
    bmp_header->bf.bfReserved1 = 0;
    bmp_header->bf.bfReserved2 = 0;
    bmp_header->bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV5HEADER);

    bmp_header->bV5.bV5Size = sizeof(BITMAPV5HEADER);
    bmp_header->bV5.bV5Width = img_header->ihWidth;
    bmp_header->bV5.bV5Height = img_header->ihHeight * -1; //-1 for upside-down image
    bmp_header->bV5.bV5Planes = 1;
    bmp_header->bV5.bV5BitCount = 16;
	bmp_header->bV5.bV5Compression = BI_BITFIELDS;
    bmp_header->bV5.bV5SizeImage = bmp_data.size();
    bmp_header->bV5.bV5XPelsPerMeter = 2835;
    bmp_header->bV5.bV5YPelsPerMeter = 2835;
    bmp_header->bV5.bV5ClrUsed = 0;
    bmp_header->bV5.bV5ClrImportant = 0;
	switch (img_header->ihBitCount) {
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
			throw runtime_error("Nieznany format kolorow pliku!\n");
		}
	}

	if (img_header->ihSizeAlpha > 0) {
		bmp_header->bV5.bV5BitCount = 32;
		bmp_header->bV5.bV5RedMask =   0x000000FF;
		bmp_header->bV5.bV5GreenMask = 0x0000FF00;
		bmp_header->bV5.bV5BlueMask =  0x00FF0000;
		bmp_header->bV5.bV5AlphaMask = 0xFF000000;
	} else if (img_header->ihCompression == 5) {
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
	cout << "Rozmiar pliku wyjsciowego w bajtach: " << (unsigned)bmp_header->bf.bfSize;
	cout << " (w tym naglowek: " << bmp_header->bf.bfOffBits << ")\n";

    return bmp_header;
}

void decompress_img(vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	char *buffer;
	CLZWCompression2 *lzw;
	int size;

	if (img_data_color.size() > 0) {
		lzw = new CLZWCompression2(img_data_color.data(), img_data_color.size());
		buffer = lzw->decompress();
		size = reinterpret_cast<int *>(img_data_color.data())[0];
		img_data_color.reserve(size);
		img_data_color.assign(buffer, buffer + size);
		delete lzw;
	}

	if (img_data_alpha.size() > 0) {
		lzw = new CLZWCompression2(img_data_alpha.data(), img_data_alpha.size());
		buffer = lzw->decompress();
		size = reinterpret_cast<int *>(img_data_alpha.data())[0];
		img_data_alpha.reserve(size);
		img_data_alpha.assign(buffer, buffer + size);
		delete lzw;
	}
}

void compress_jpg(vector<char> &img_data_color, IMGHEADER *img_header)
{
	tjhandle compressor = tjInitCompress();
	if (compressor != nullptr) {
		unsigned long buffer_size = tjBufSize(img_header->ihWidth, img_header->ihHeight, TJSAMP_444);
		char *buffer = new char[buffer_size];
		if (!tjCompress2(compressor, (const unsigned char *)(img_data_color.data()), img_header->ihWidth, 0, img_header->ihHeight,
						 TJPF_RGB, (unsigned char **)&buffer, &buffer_size, TJSAMP_444, 90, TJFLAG_NOREALLOC)) {
			img_data_color.assign(buffer, buffer + buffer_size);
			tjDestroy(compressor);
		} else {
			tjDestroy(compressor);
			throw runtime_error(tjGetErrorStr2(compressor));
		}
	} else {
		throw runtime_error(tjGetErrorStr2(compressor));
	}
}

void decompress_jpg(vector<char> &img_data_color, IMGHEADER *img_header, enum output_format format)
{
	tjhandle decompressor = tjInitDecompress();
	if (decompressor != nullptr) {
		unsigned buffer_size = img_header->ihWidth * img_header->ihHeight * (img_header->ihSizeAlpha > 0 ? 4 : 3);
		char *buffer = new char[buffer_size];
		int pixel_format;
		if (img_header->ihSizeAlpha > 0) {
			pixel_format = TJPF_RGBA;
		} else {
			if (format == BMP) {
				pixel_format = TJPF_BGR;
			} else {
				pixel_format = TJPF_RGB;
			}
		}
		if (!tjDecompress2(decompressor, (const unsigned char *)(img_data_color.data()), img_data_color.size(), (unsigned char *)(buffer),
						  img_header->ihWidth, img_header->ihWidth * (img_header->ihSizeAlpha > 0 ? 4 : 3), img_header->ihHeight,
						  pixel_format, 0)) {
			img_data_color.assign(buffer, buffer + buffer_size);
			tjDestroy(decompressor);
		} else {
			tjDestroy(decompressor);
			throw runtime_error(tjGetErrorStr2(decompressor));
		}
	} else {
		throw runtime_error(tjGetErrorStr2(decompressor));
	}
}

vector<char> prepare_bmp_data(IMGHEADER *img_header, vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	switch (img_header->ihCompression) {
		case 2: {
			decompress_img(img_data_color, img_data_alpha);
			break;
		}
		case 5: {
			decompress_jpg(img_data_color, img_header, BMP);
			decompress_img(vector<char>(0), img_data_alpha);
			break;
		}
	}
	vector<char> aligned_bmp_data(0);
	char *buffer = (char *)(img_data_color.data());
	unsigned buffer_size = img_data_color.size();
	if (img_header->ihSizeAlpha == 0) {
		if (img_header->ihBitCount == 15) {
			buffer_size = img_data_color.size();
			buffer = new char[buffer_size];
			copy(img_data_color.begin(), img_data_color.end(), buffer);
			#ifdef _DEBUG
				clog << "[log] Skopiowano bufor danych obrazu!\n";
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
				clog << "[log] Przetworzono bufor danych obrazu!\n";
			#endif
			//go to padding check
		} else if (img_header->ihBitCount != 16 && img_header->ihBitCount != 5) { //for 16bpp go straight to padding check
			throw runtime_error("Nieznany format kolorow pliku!\n");
		}
	} else {
		unsigned pixel_count = img_header->ihWidth * img_header->ihHeight;
		if (img_data_color.size() / 2 == img_data_alpha.size()) {
			buffer_size = pixel_count * 4;
			buffer = new char[buffer_size];
			char pixel[4];
			if (img_header->ihBitCount == 16) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
					pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			} else if (img_header->ihBitCount == 15) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
					pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			} else {
				throw runtime_error("Nieznany format kolorow pliku!\n");
			}
		} else if (img_data_color.size() / 4 == img_data_alpha.size() && img_header->ihCompression == 5) {
			for (unsigned i = 0; i < pixel_count; i++) {
				buffer[i * 4 + 3] = img_data_alpha[i];
			}
		} else {
			img_header->ihSizeAlpha = 0;
			throw runtime_error("Nieznany format alfy pliku!\n");
		}
	}

	if (buffer_size / img_header->ihHeight % 4 > 0) {
		unsigned row_length = buffer_size / img_header->ihHeight;
		unsigned padding_length = 4 - row_length % 4;
		unsigned padded_row_length = row_length + padding_length;
		unsigned padded_buffer_size = padded_row_length * img_header->ihHeight;
		char padding[4] = {0, 0, 0, 0};
		char *padded_buffer = new char[padded_buffer_size];
		for (int i = 0; i < img_header->ihHeight; i++) {
			copy(buffer + i * row_length, buffer + (i + 1) * row_length, padded_buffer + i * padded_row_length);
			copy(padding, padding + padding_length, padded_buffer + i * padded_row_length + row_length);
		}
		#ifdef _DEBUG
			clog << "[log] Przetworzono bufor danych obrazu!\n";
		#endif
		aligned_bmp_data.reserve(padded_buffer_size);
		aligned_bmp_data.assign(padded_buffer, padded_buffer + padded_buffer_size);
		delete[] padded_buffer;
	} else {
		#ifdef _DEBUG
			clog << "[log] Przetworzono bufor danych obrazu!\n";
		#endif
		aligned_bmp_data.reserve(buffer_size);
		aligned_bmp_data.assign(buffer, buffer + buffer_size);
	}

	if (buffer != img_data_color.data()) {
		delete[] buffer;
	}
	return aligned_bmp_data;
}

vector<char> prepare_jpg_data(IMGHEADER *img_header, vector<char> &img_data_color)
{
	if (img_header->ihCompression != 5) {
		if (img_header->ihCompression == 2) {
			decompress_img(img_data_color, vector<char>(0));
		}
		unsigned pixel_count = img_header->ihWidth * img_header->ihHeight;
		unsigned buffer_size = pixel_count * 3;
		char *buffer = new char[buffer_size];
		char pixel[3];
		if (img_header->ihBitCount == 15) {
			for (unsigned i = 0; i < pixel_count; i++) {
				pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
				pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
				pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
				copy(pixel, pixel + 3, buffer + i * 3);
			}
		} else if (img_header->ihBitCount == 16) {
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
		return vector<char>(0);
	} else {
		return vector<char>(0);
	}
}

vector<char> prepare_png_data(IMGHEADER *img_header, vector<char> &img_data_color, vector<char> &img_data_alpha)
{
	vector<char> png_data(0);
	if (img_header->ihCompression != 5) {
		if (img_header->ihCompression == 2) {
			decompress_img(img_data_color, img_data_alpha);
		}
		unsigned pixel_count = img_header->ihWidth * img_header->ihHeight;
		unsigned buffer_size = pixel_count * (img_header->ihSizeAlpha > 0 ? 4 : 3);
		char *buffer = new char[buffer_size];
		if (img_header->ihSizeAlpha == 0) {
			char pixel[3];
			if (img_header->ihBitCount == 15) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
					pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					copy(pixel, pixel + 3, buffer + i * 3);
				}
			} else if (img_header->ihBitCount == 16) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
					pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					copy(pixel, pixel + 3, buffer + i * 3);
				}
			}
		} else if (img_data_color.size() / 2 == img_data_alpha.size()) {
			char pixel[4];
			if (img_header->ihBitCount == 15) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0x7C) >> 2); //red
					pixel[1] = 255 / 31. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x03) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			} else if (img_header->ihBitCount == 16) {
				for (unsigned i = 0; i < pixel_count; i++) {
					pixel[0] = 255 / 31. * ((img_data_color[i * 2 + 1] & 0xF8) >> 3); //red
					pixel[1] = 255 / 63. * (((img_data_color[i * 2] & 0xE0) >> 5) | ((img_data_color[i * 2 + 1] & 0x07) << 3)); //green
					pixel[2] = 255 / 31. * (img_data_color[i * 2] & 0x1F); //blue
					pixel[3] = img_data_alpha[i]; //alpha
					copy(pixel, pixel + 4, buffer + i * 4);
				}
			}
		} else {
			img_header->ihSizeAlpha = 0;
			throw runtime_error("Nieznany format alfy pliku!\n");
		}
		img_data_color.assign(buffer, buffer + buffer_size);
		delete[] buffer;
	} else {
		decompress_jpg(img_data_color, img_header, PNG);
		decompress_img(vector<char>(0), img_data_alpha);
		if (img_data_color.size() / 4 == img_data_alpha.size()) {
			unsigned pixel_count = img_header->ihWidth * img_header->ihHeight;
			for (unsigned i = 0; i < pixel_count; i++) {
				img_data_color[i * 4 + 3] = img_data_alpha[i];
			}
		} else if (img_header->ihSizeAlpha > 0) {
			img_header->ihSizeAlpha = 0;
			throw runtime_error("Nieznany format alfy pliku!\n");
		}
	}
	unsigned buffer_size;
	char *buffer;
	buffer = (char *)tdefl_write_image_to_png_file_in_memory_ex(img_data_color.data(), img_header->ihWidth, img_header->ihHeight,
																(img_header->ihSizeAlpha > 0 ? 4 : 3), &buffer_size, MZ_BEST_COMPRESSION, false);
	if (buffer != nullptr && buffer_size != 0) {
		//cout << "Przekonwertowano do PNG (nowy rozmiar w bajtach: " << buffer_size << ")\n";
		png_data.assign(buffer, buffer + buffer_size);
		mz_free(buffer);
	} else {
		throw runtime_error("Nie udalo sie skompresowac obrazu do PNG!\n");
	}
	return png_data;
}

void write_bmp(ofstream &bmp_file, IMGHEADER *img_header, const vector<char> &bmp_data)
{
	BITMAPHEADER *bmp_header = prepare_bmp_header(img_header, bmp_data);
	bmp_file.write((char *)(bmp_header), sizeof(BITMAPHEADER));
	bmp_file.write(bmp_data.data(), bmp_data.size());
}

void write_jpg(ofstream &jpg_file, IMGHEADER *img_header, const vector<char> &jpg_data)
{
	jpg_file.write(jpg_data.data(), jpg_data.size());
}

void write_png(ofstream &png_file, IMGHEADER *img_header, const vector<char> &png_data)
{
	png_file.write(png_data.data(), png_data.size());
}

void print_help()
{
	cout << "Sposob uzycia:\n"
			"  dePIKczer [opcje] nazwa_pliku1[, nazwa_pliku2, ...]\n"
			"Dostepne opcje (nalezy poprzedzic znakiem / lub -):\n"
			"  d,                  dekompresja pliku IMG (opcja domyslna)\n"
			"  decompress\n"
			"  c,                  kompresja pliku BMP, JPG lub PNG do IMG\n"
			"  compress              (funkcjonalnosc niezaimplementowana)\n"
			"  f [format],         uzycie podanego formatu jako formatu wyjsciowego\n"
			"  out-format=[format]   tylko dla dekompresji\n"
			"                        dostepne formaty:\n"
			"                          bmp,\n"
			"                          jpg, jpeg,\n"
			"                          png (domyslny)\n"
			"  h,                  wyswietlenie tego tekstu pomocy\n"
			"  help\n"
			"  o [sciezka],        uzycie podanego katalogu jako zbiorczego katalogu\n"
			"  out-dir=[sciezka]     wyjsciowego dla przetworzonych plikow\n"
			"                        (domyslnie przetworzone pliki zapisywane sa w tym\n"
			"                        samym katalogu, co pliki wejsciowe)\n"
			"  s,                  wymuszenie wyciszenia programu (braku konsoli)\n"
			"  silent\n";
}

int main(int argc, char **argv)
{
	push_back_characteristic_names();
	int starting_argument = 1;
	if (argc > 0) {
		if (argv[0][0] == '-' || argv[0][0] == '/') {
			starting_argument--;
		}
	}
    if (argc > starting_argument) {
		int arg_iter = starting_argument;
		cli_options options;
		options.decompress = true;
		options.format = PNG;
		options.custom_dir = false;
		options.silence = false;
		options.parsed_without_error = false;
		try {
			parse_cli_options(argc, argv, options, arg_iter);
			options.parsed_without_error = true;
		} catch (invalid_argument &e) {
			console_writer::get_instance(argc);
			cerr << e.what();
			cerr << "\tprzy argumencie " << argv[arg_iter] << '\n';
		} catch (out_of_range &e) {
			console_writer::get_instance(argc);
			cerr << e.what();
			cerr << "\tdla argumentu " << argv[arg_iter] << '\n';
		} catch (help_issued &e) {
			console_writer::get_instance(argc);
			print_help();
		}
		if (options.parsed_without_error) {
			console_writer::get_instance(argc, options.silence);
			for (; arg_iter < argc; arg_iter++) {
				ifstream in_file(argv[arg_iter], ios::in | ios::binary);
				if (in_file.good()) {
					cout << "Przetwarzanie pliku " << argv[arg_iter] << "...\n";
					IMGHEADER *img_header = read_img_header(in_file);
					if (img_header != nullptr) {
						cout << "Wczytano naglowek!\n";
						cout << "Wymiary obrazu: " << img_header->ihWidth << " x " << abs(img_header->ihHeight) << " px\n";

						vector<char> img_data_color, img_data_alpha;
						read_img_data(in_file, img_header, img_data_color, img_data_alpha);
						if (img_data_color.size() != 0) {
							cout << "Wczytano dane obrazu!\n";
							cout << "Rozmiar w bajtach: " << img_data_color.size() + img_data_alpha.size();
							if (img_data_alpha.size() > 0) {
								cout << " (w tym alpha: " << img_data_alpha.size() << ')';
							}
							cout << '\n';

							string out_filename;
							bool correct_dir = false;
							try {
								if (options.custom_dir) {
									if (!CreateDirectoryA(options.dir.c_str(), 0)) {
										unsigned err = GetLastError();
										if (err == ERROR_PATH_NOT_FOUND) {
											throw invalid_argument("Nieprawidlowa sciezka katalogu wyjsciowego!\n");
										} else if (err != ERROR_ALREADY_EXISTS) {
											throw runtime_error(to_string(_ULonglong(err)));
										}
									}
									out_filename = options.dir;
									if (out_filename.back() != '\\') {
										out_filename.append("\\");
									}
									compose_out_filename(out_filename, argv, arg_iter, options.format);
								} else {
									out_filename = argv[arg_iter];
									if (out_filename[out_filename.size() - 4] == '.') {
										out_filename.erase(out_filename.size() - 4);
									}
									switch (options.format) {
										case BMP: {
											out_filename.append(".bmp");
											break;
										}
										case JPG: {
											out_filename.append(".jpg");
											break;
										}
										case PNG: {
											out_filename.append(".png");
											break;
										}
									}
								}
								correct_dir = true;
							} catch (exception &e) {
								cerr << "Blad przy tworzeniu katalogu wyjsciowego: " << e.what() << '\n';
							}
							if (correct_dir) {
								ofstream out_file(out_filename, ios::out | ios::binary);
								if (out_file.good()) {
									try {
										switch (options.format) {
											case BMP: {
												vector<char> bmp_data = prepare_bmp_data(img_header, img_data_color, img_data_alpha);
												cout << "Przekonwertowano do BMP!\n";
												cout << "Nowy rozmiar w bajtach: "
													 << (bmp_data.size() != 0 ? bmp_data.size() : img_data_color.size()) << '\n';
												if (bmp_data.size() > 0) {
													write_bmp(out_file, img_header, bmp_data);
												} else {
													write_bmp(out_file, img_header, img_data_color);
												}
												break;
											}
											case JPG: {
												vector<char> jpg_data = prepare_jpg_data(img_header, img_data_color);
												cout << "Przekonwertowano do JPG!\n";
												cout << "Nowy rozmiar w bajtach: "
													 << (jpg_data.size() != 0 ? jpg_data.size() : img_data_color.size()) << '\n';
												if (jpg_data.size() > 0) {
													write_jpg(out_file, img_header, jpg_data);
												} else {
													write_jpg(out_file, img_header, img_data_color);
												}
												break;
											}
											case PNG: {
												vector<char> png_data = prepare_png_data(img_header, img_data_color, img_data_alpha);
												cout << "Przekonwertowano do PNG!\n";
												cout << "Nowy rozmiar w bajtach: " << png_data.size() << '\n';
												write_png(out_file, img_header, png_data);
												break;
											}
										}
										cout << "Zapisano do pliku " << out_filename << "!\n";
									} catch (exception &e) {
										cerr << "Nie udalo sie dokonac konwersji!\n";
										cerr << e.what();
									}
									out_file.close();
								} else {
									cerr << "Blad pliku: " << strerror(errno) << " dla pliku wyjscia " << out_filename << '\n';
								}
							}
						} else {
							cerr << "Blad wczytywania danych obrazu!\n";
						}
						delete img_header;
					} else {
						cerr << "Blad wczytywania naglowka pliku!\n";
					}
				} else {
					cerr << "Blad pliku: " << strerror(errno) << " dla pliku wejscia " << argv[arg_iter] << '\n';
				}
				in_file.close();
				cout << '\n';
			}
		}
    } else {
		console_writer::get_instance(argc);
		cerr << "Blad: Nie podano plikow!\n\n";
        print_help();
    }
    return 0;
}
