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
    bmp_header->bV5.bV5Height = img_header->ihHeight;
    bmp_header->bV5.bV5Planes = 1;
    bmp_header->bV5.bV5BitCount = 16;
    
	bmp_header->bV5.bV5Compression = BI_BITFIELDS;
    bmp_header->bV5.bV5Height *= -1;
    bmp_header->bV5.bV5AlphaMask = 0;
	switch (img_header->ihBitCount) {
		case 15: {
		}
		case 16: {
			bmp_header->bV5.bV5RedMask =   0xF800;
			bmp_header->bV5.bV5GreenMask = 0x07E0;
			bmp_header->bV5.bV5BlueMask =  0x001F;
			break;
		}
		default: {
			delete bmp_header;
			throw runtime_error("Nieznany format kolorow pliku!\n");
		}
	}

    bmp_header->bV5.bV5SizeImage = bmp_data.size();
	/*if (img_header->ihSizeAlpha > 0) {
		bmp_header->bV5.bV5BitCount = 32;
		bmp_header->bV5.bV5Compression = BI_BITFIELDS;
		bmp_header->bV5.bV5RedMask =   0x000000FF;
		bmp_header->bV5.bV5GreenMask = 0x0000FF00;
		bmp_header->bV5.bV5BlueMask =  0x00FF0000;
		bmp_header->bV5.bV5AlphaMask = 0xFF000000;
	}*/
    bmp_header->bV5.bV5XPelsPerMeter = 2835;
    bmp_header->bV5.bV5YPelsPerMeter = 2835;
    bmp_header->bV5.bV5ClrUsed = 0;
    bmp_header->bV5.bV5ClrImportant = 0;
    bmp_header->bV5.bV5CSType = LCS_sRGB;
    bmp_header->bV5.bV5Endpoints = tagICEXYZTRIPLE();
    bmp_header->bV5.bV5GammaRed = 0;
    bmp_header->bV5.bV5GammaGreen = 0;
    bmp_header->bV5.bV5GammaBlue = 0;
    bmp_header->bV5.bV5Intent = 0;
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

	lzw = new CLZWCompression2(img_data_color.data(), img_data_color.size());
	buffer = lzw->decompress();
	size = reinterpret_cast<int *>(img_data_color.data())[0];
	img_data_color.reserve(size);
	img_data_color.assign(buffer, buffer + size);
	delete lzw;

	if (img_data_alpha.size() > 0) {
		lzw = new CLZWCompression2(img_data_alpha.data(), img_data_alpha.size());
		buffer = lzw->decompress();
		size = reinterpret_cast<int *>(img_data_alpha.data())[0];
		img_data_alpha.reserve(size);
		img_data_alpha.assign(buffer, buffer + size);
		delete lzw;
	}
}

vector<char> align_bmp_data(IMGHEADER *img_header, const vector<char> &img_data_color, const vector<char> &img_data_alpha)
{
	if (img_header->ihBitCount == 16) {
		//if (img_header->ihSizeAlpha == 0) {
			if (img_header->ihWidth % 2 == 0) {
				return vector<char>(0);
			} else {
				unsigned row_length = img_header->ihWidth * 2;
				unsigned padding_length = 4 - row_length % 4;
				unsigned padded_row_length = row_length + padding_length;
				unsigned buffer_size = padded_row_length * img_header->ihHeight;
				char padding[4] = {0, 0, 0, 0};
				char *buffer = new char[buffer_size];
				for (int i = 0; i < img_header->ihHeight; i++) {
					copy(img_data_color.data() + i * row_length, img_data_color.data() + (i + 1) * row_length, buffer + i * padded_row_length);
					copy(padding, padding + padding_length, buffer + i * padded_row_length + row_length);
				}
				return vector<char>(buffer, buffer + buffer_size);
			}
		/*} else {
			if (img_data_color.size() / 2 == img_data_alpha.size()) {
				;
			} else {
				throw runtime_error("Nieznany format alfy pliku!\n");;
			}
		}*/
	} else if (img_header->ihBitCount == 15) {
		return vector<char>(0);
	} else {
		throw runtime_error("Nieznany format kolorow pliku!\n");
	}
}

void write_bmp(ofstream &bmp_file, IMGHEADER *img_header, const vector<char> &bmp_data)
{
	BITMAPHEADER *bmp_header = prepare_bmp_header(img_header, bmp_data);
	bmp_file.write((char *)(bmp_header), sizeof(BITMAPHEADER));
	bmp_file.write(bmp_data.data(), bmp_data.size());
	/*if (bmp_header->bV5.bV5BitCount == 32) {
		char buffer[4];
		buffer[3] = 0;
		short color_pixel;
		for (int i = 0; i < img_header->ihWidth * img_header->ihHeight; i++) {
			bmp_file.write(img_data_alpha.data() + i, 1);
			color_pixel = reinterpret_cast<short>(img_data_color.data() + i * 2);
			buffer[0] = 255 / 31. * (color_pixel & 0x001F);
			buffer[1] = 255 / 64. * ((color_pixel & 0x07E0) >> 5);
			buffer[2] = 255 / 31. * ((color_pixel & 0xF800) >> 11);
			bmp_file.write(buffer, 4);
		}
	} else {
		if (img_header->ihWidth * img_header->ihBitCount % 32 == 0) {
			bmp_file.write(img_data_color.data(), img_data_color.size());
		} else {//DWORD alignment
			unsigned row_length = img_header->ihWidth * img_header->ihBitCount / 8;
			unsigned padding_length = 4 - row_length % 4;
			char padding[4] = {0, 0, 0, 0};
			for (int i = 0; i < img_header->ihHeight; i++) {
				bmp_file.write(img_data_color.data() + i * img_header->ihBitCount / 8, row_length);
				bmp_file.write(padding, padding_length);
			}
		}
	}*/
}

void write_jpg(ofstream &jpg_file, const vector<char> &img_data_color)
{
	jpg_file.write(img_data_color.data(), img_data_color.size());
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        for (int arg_iter = 1; arg_iter < argc; arg_iter++) {
            ifstream in_file(argv[arg_iter], ios::in | ios::binary);
            if (in_file.good()) {
                IMGHEADER *img_header = read_img_header(in_file);
                if (img_header != nullptr) {
                    cout << "Wczytano naglowek!\n";
                    cout << "Rozmiar obrazu: " << img_header->ihWidth << " x " << abs(img_header->ihHeight) << " px\n";

					vector<char> img_data_color, img_data_alpha;
					read_img_data(in_file, img_header, img_data_color, img_data_alpha);
					if (img_data_color.size() != 0) {
						cout << "Wczytano dane obrazu!\n";
						cout << "Rozmiar w bajtach: " << img_data_color.size() + img_data_alpha.size();
						if (img_data_alpha.size() > 0) {
							cout << " (w tym alpha: " << img_data_alpha.size() << ')';
						}
						cout << '\n';

						string out_filename = argv[arg_iter];
						if (img_header->ihCompression != 5) {
							out_filename += string(".bmp");
						} else {
							out_filename += string(".jpg");
						}
						ofstream out_file(out_filename, ios::out | ios::binary);

						if (out_file.good()) {
							if (img_header->ihCompression != 5) {
								if (img_header->ihCompression == 2) {
									decompress_img(img_data_color, img_data_alpha);
								}
								cout << "Zdekompresowano dane obrazu!\n";
								cout << "Nowy rozmiar w bajtach: " << img_data_color.size() + img_data_alpha.size();
								if (img_data_alpha.size() > 0) {
									cout << " (w tym alpha: " << img_data_alpha.size() << ')';
								}
								cout << '\n';
								vector<char> bmp_data;
								try {
									bmp_data = align_bmp_data(img_header, img_data_color, img_data_alpha);
									cout << "Przekonwertowano do formatu BMP!\n";
									if (bmp_data.size() > 0) {
										write_bmp(out_file, img_header, bmp_data);
									} else {
										write_bmp(out_file, img_header, img_data_color);
									}
									cout << "Zapisano plik!\n";
								} catch (exception &e) {
									cerr << e.what();
									cerr << "Nie udalo sie dokonac kompresji!\n";
								}
							} else {
								write_jpg(out_file, img_data_color);
								cout << "Przekonwertowano do formatu JPG!\n";
								cout << "Zapisano plik!\n";
							}
							out_file.close();
						} else {
							cerr << "Blad pliku: " << strerror(errno) << " dla pliku wyjscia " << out_filename << '\n';
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
    } else {
        cout << "Sposob uzycia:\n";
		cout << "\tdePIKczer nazwa_pliku1[, nazwa_pliku2, ...]\n";
		cerr << "Nie podano argumentow!\n";
    }
    return 0;
}
