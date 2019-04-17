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
    int ihWidth;
    int ihHeight;
    int ihBitCount;
    int ihSizeImage;
    int something;
    int ihCompresion;
    int ihSizeAlpha;
    int pos_x;
    int pos_y;
};

struct BITMAPFILEHEADER {
    unsigned short filler;
    unsigned short bfType;
    unsigned long bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long bfOffBits;
};

struct BITMAPV4HEADER {
    unsigned long bV4Size;
    long bV4Width;
    long bV4Height;
    unsigned short bV4Planes;
    unsigned short bV4BitCount;
    unsigned long bV4Compression;
    unsigned long bV4SizeImage;
    long bV4XPelsPerMeter;
    long bV4YPelsPerMeter;
    unsigned long bV4ClrUsed;
    unsigned long bV4ClrImportant;
    unsigned long bV4RedMask;
    unsigned long bV4GreenMask;
    unsigned long bV4BlueMask;
    unsigned long bV4AlphaMask;
    unsigned long bV4CSType;
    //CIEXYZ
    long RedX;
    long RedY;
    long RedZ;
    long GreenX;
    long GreenY;
    long GreenZ;
    long BlueX;
    long BlueY;
    long BlueZ;
    //
    unsigned long bV4GammaRed;
    unsigned long bV4GammaGreen;
    unsigned long bV4GammaBlue;
};

struct BITMAPHEADER {
    struct BITMAPFILEHEADER bf;
    struct BITMAPV4HEADER bV4;
};

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

struct IMGHEADER *read_img_header(ifstream &img_file)
{
    iostream::pos_type init_pos;
	init_pos = img_file.tellg();
	img_file.seekg(0, ios::beg);

    IMGHEADER *img_header = new IMGHEADER;
	if (img_file.read((char *)(img_header), sizeof(IMGHEADER))) {
		if (img_header->ihType == string("PIK")) {
			cout << "Poprawny typ pliku\n";
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

struct BITMAPHEADER *prepare_bmp_header(IMGHEADER *img_header)
{
    BITMAPHEADER *bmp_header = new BITMAPHEADER;

    bmp_header->bf.bfType = 0x4D42;
    bmp_header->bf.bfReserved1 = 0;
    bmp_header->bf.bfReserved2 = 0;
    bmp_header->bf.bfOffBits = sizeof(BITMAPFILEHEADER) - 2 + sizeof(BITMAPV4HEADER);

    bmp_header->bV4.bV4Size = sizeof(BITMAPV4HEADER);
    bmp_header->bV4.bV4Width = img_header->ihWidth;
    bmp_header->bV4.bV4Height = img_header->ihHeight;
    bmp_header->bV4.bV4Planes = 1;
    bmp_header->bV4.bV4BitCount = img_header->ihBitCount;
    bmp_header->bV4.bV4RedMask = 0;
    bmp_header->bV4.bV4GreenMask = 0;
    bmp_header->bV4.bV4BlueMask = 0;
    switch (img_header->ihCompresion) {
        case 0: {
            cout << "Wykryto obraz bez kompresji!\n";
            break;
        }
        case 2: {
            cout << "Wykryto kompresje CLZW2!\n";
            break;
        }
        case 4: {
            cout << "Format nr 4 ;-;\n";
			break;
        }
        case 5: {
            cout << "To JPG, a nie BMP. Zapomnij, co widziales.\n";
            break;
        }
        default: {
            cout << "Niestandardowy format pliku .img!\n";
            break;
        }
    }
	bmp_header->bV4.bV4Compression = 3; //BI_BITFIELDS
    bmp_header->bV4.bV4Height *= -1;
    bmp_header->bV4.bV4AlphaMask = 0;
    bmp_header->bV4.bV4RedMask = 0xF800;
    bmp_header->bV4.bV4GreenMask = 0x7E0;
    bmp_header->bV4.bV4BlueMask = 0x1F;

    bmp_header->bV4.bV4SizeImage = img_header->ihSizeImage;
    bmp_header->bV4.bV4XPelsPerMeter = 2835;
    bmp_header->bV4.bV4YPelsPerMeter = 2835;
    bmp_header->bV4.bV4ClrUsed = 0;
    bmp_header->bV4.bV4ClrImportant = 0;
    bmp_header->bV4.bV4CSType = 0x73524742; //sRGB
    bmp_header->bV4.RedX = 0;
    bmp_header->bV4.RedY = 0;
    bmp_header->bV4.RedZ = 0;
    bmp_header->bV4.GreenX = 0;
    bmp_header->bV4.GreenY = 0;
    bmp_header->bV4.GreenZ = 0;
    bmp_header->bV4.BlueX = 0;
    bmp_header->bV4.BlueY = 0;
    bmp_header->bV4.BlueZ = 0;
    bmp_header->bV4.bV4GammaRed = 0;
    bmp_header->bV4.bV4GammaGreen = 0;
    bmp_header->bV4.bV4GammaBlue = 0;

    bmp_header->bf.bfSize = bmp_header->bf.bfOffBits + bmp_header->bV4.bV4SizeImage;
	cout << "Rozmiar pliku wyjsciowego: " << (unsigned)bmp_header->bf.bfSize << '\n';

    return bmp_header;
}

void decompress_bmp(string &buffer, ifstream &img_file, IMGHEADER *img_header, BITMAPHEADER *bmp_header)
{
	iostream::pos_type init_pos = img_file.tellg();
    img_file.seekg(40, ios::beg);

	char *in_buffer = new char[img_header->ihSizeImage];
	img_file.read(in_buffer, img_header->ihSizeImage);
	CLZWCompression2 lzw(in_buffer, img_header->ihSizeImage);
	char *out_buffer = lzw.decompress();
	bmp_header->bV4.bV4SizeImage = reinterpret_cast<int *>(in_buffer)[0];
    bmp_header->bf.bfSize = bmp_header->bf.bfOffBits + bmp_header->bV4.bV4SizeImage;
	cout << "Rozmiar pliku wyjsciowego po dekompresji: " << (unsigned)bmp_header->bf.bfSize << '\n';
	buffer.reserve(bmp_header->bV4.bV4SizeImage);
	buffer.assign(out_buffer, bmp_header->bV4.bV4SizeImage);
	delete[] in_buffer;

    img_file.seekg(init_pos);
}

void write_bmp(ofstream &bmp_file, BITMAPHEADER *bmp_header, ifstream &img_file)
{
    iostream::pos_type init_pos = img_file.tellg();
	img_file.seekg(40, ios::beg);

	bmp_file.write((char *)(&bmp_header->bf.bfType), sizeof(BITMAPFILEHEADER) - 2);
	bmp_file.write((char *)(&bmp_header->bV4), sizeof(BITMAPV4HEADER));
    char buffer[BUFF_SIZE];
    while (!img_file.eof()) {
		img_file.read(buffer, BUFF_SIZE);
		bmp_file.write(buffer, img_file.gcount());
    }

    img_file.seekg(init_pos);
}

void write_bmp(ofstream &bmp_file, BITMAPHEADER *bmp_header, string &data_buffer)
{
	bmp_file.write((char *)(&bmp_header->bf.bfType), sizeof(BITMAPFILEHEADER) - 2);
	bmp_file.write((char *)(&bmp_header->bV4), sizeof(BITMAPV4HEADER));
    bmp_file.write(data_buffer.c_str(), data_buffer.size());
}

void write_jpg(ofstream &jpg_file, ifstream &img_file)
{
	iostream::pos_type init_pos = img_file.tellg();
	img_file.seekg(40, ios::beg);

    char buffer[BUFF_SIZE];
    while (!img_file.eof()) {
		img_file.read(buffer, BUFF_SIZE);
		jpg_file.write(buffer, img_file.gcount());
    }

    img_file.seekg(init_pos);
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        for (int arg_iter = 1; arg_iter < argc; arg_iter++) {
            ifstream in_file(argv[arg_iter], ios::in | ios::binary);
            if (in_file.good()) {
                IMGHEADER *img_header = read_img_header(in_file);
                if (img_header != nullptr) {
                    BITMAPHEADER *bmp_header;

                    cout << "Wczytano naglowek!\n";
                    cout << "Rozmiar obrazu: " << img_header->ihWidth << " x " << abs(img_header->ihHeight) << " px\n";
					
					string out_filename = argv[arg_iter];
					if (img_header->ihCompresion != 5) {
						bmp_header = prepare_bmp_header(img_header);
						out_filename += string(".bmp");
					} else {
						out_filename += string(".jpg");
					}
					ofstream out_file(out_filename, ios::out | ios::binary);

					if (out_file.good()) {
						switch (img_header->ihCompresion) {
							case 4: {
								//?
							}
							case 0: {
								write_bmp(out_file, bmp_header, in_file);
								break;
							}
							case 2: {
								//CLZW2
								string buffer;
								decompress_bmp(buffer, in_file, img_header, bmp_header);
								write_bmp(out_file, bmp_header, buffer);
								break;
							}
							case 5: {
								write_jpg(out_file, in_file);
								break;
							}
							default: {
								cout << "Niestandardowy format pliku .img!\n";
								break;
							}
						}
 
						if (img_header->ihCompresion != 5) {
							cout << "Przekonwertowano do formatu BMP!\n";
						} else {
							cout << "Przekonwertowano do formatu JPG!\n";
						}
					} else {
						cerr << "File error: " << strerror(errno) << " for output file " << out_filename << '\n';
					}

					if (img_header->ihCompresion != 5) {
						delete bmp_header;
					}
                    delete img_header;
					out_file.close();
                }
            } else {
                cerr << "File error: " << strerror(errno) << " for input file " << argv[arg_iter] << '\n';
            }
			in_file.close();
			cout << '\n';
        }
    } else {
        cerr << "Podaj nazwe pliku jako argument!\n";
    }
    return 0;
}
