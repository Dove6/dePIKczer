#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 200

struct IMGHEADER {
    char ihType[4];
    int ihWidth;
    int ihHeight;
    int ihBitCount;
    int ihSizeImage;
    int something;
    int ihCompresion;
    int something2;
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

struct IMGHEADER *read_img_header(FILE *img_file)
{
    fpos_t init_pos;
    fgetpos(img_file, &init_pos);
    fseek(img_file, 0, SEEK_SET);

    struct IMGHEADER *img_header = malloc(sizeof(struct IMGHEADER));
    fread(img_header, 1, sizeof(struct IMGHEADER), img_file);
    if (!strcmp(img_header->ihType, "PIK")) {
        puts("Woo");
        return img_header;
    } else {
        puts("Nieprawidlowy format pliku!");
    }

    free(img_header);
    fsetpos(img_file, &init_pos);
    return NULL;
}

struct BITMAPHEADER *prepare_bmp_header(struct IMGHEADER *img_header)
{
    struct BITMAPHEADER *bmp_header = malloc(sizeof(struct BITMAPHEADER));

    bmp_header->bf.bfType = 0x4D42;
    bmp_header->bf.bfReserved1 = 0;
    bmp_header->bf.bfReserved2 = 0;
    bmp_header->bf.bfOffBits = sizeof(struct BITMAPFILEHEADER) - 2 + sizeof(struct BITMAPV4HEADER);

    bmp_header->bV4.bV4Size = sizeof(struct BITMAPV4HEADER);
    bmp_header->bV4.bV4Width = img_header->ihWidth;
    bmp_header->bV4.bV4Height = -img_header->ihHeight;
    bmp_header->bV4.bV4Planes = 1;
    bmp_header->bV4.bV4BitCount = img_header->ihBitCount;
    bmp_header->bV4.bV4Compression = 3;
    bmp_header->bV4.bV4SizeImage = img_header->ihSizeImage;
    bmp_header->bV4.bV4XPelsPerMeter = 2835;
    bmp_header->bV4.bV4YPelsPerMeter = 2835;
    bmp_header->bV4.bV4ClrUsed = 0;
    bmp_header->bV4.bV4ClrImportant = 0;
    bmp_header->bV4.bV4RedMask = 0xF800;
    bmp_header->bV4.bV4GreenMask = 0x7E0;
    bmp_header->bV4.bV4BlueMask = 0x1F;
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
    printf("%u\n", (unsigned)bmp_header->bf.bfSize);

    return bmp_header;
}

void write_bmp(FILE *bmp_file, struct BITMAPHEADER *bmp_header, FILE *img_file)
{
    fpos_t init_pos;
    fgetpos(img_file, &init_pos);
    fseek(img_file, 0, SEEK_SET);

    //fwrite(&bmp_header->bf.bfType, 1, sizeof(unsigned short), bmp_file);
    fwrite(&bmp_header->bf.bfType, 1, sizeof(struct BITMAPFILEHEADER) - 2, bmp_file);
    fwrite(&bmp_header->bV4, 1, sizeof(struct BITMAPV4HEADER), bmp_file);
    fseek(img_file, 40, SEEK_SET);
    char buffer[BUFF_SIZE];
    unsigned buff_read;
    while (!feof(img_file)) {
        buff_read = fread(buffer, 1, BUFF_SIZE, img_file);
        fwrite(buffer, 1, buff_read, bmp_file);
    }

    fsetpos(img_file, &init_pos);
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        for(int arg_iter=1;arg_iter<argc;arg_iter++){
            char *out_filename = malloc(strlen(argv[arg_iter]) + 5);
            strcpy(out_filename, argv[arg_iter]);
            strcat(out_filename, ".bmp");
            //puts(out_filename);
            FILE *in_file = fopen(argv[arg_iter], "rb"), *out_file = fopen(out_filename, "wb");
            free(out_filename);
            if (in_file != NULL && out_file != NULL) {
                struct IMGHEADER *img_header = read_img_header(in_file);
                if (img_header != NULL) {
                    struct BITMAPHEADER *bmp_header = prepare_bmp_header(img_header);

                    puts("Wczytano naglowek!");
                    printf("Rozmiar obrazu: %d x %d px\n", img_header->ihWidth, abs(img_header->ihHeight));

                    write_bmp(out_file, bmp_header, in_file);
                    puts("Przekonwertowano do formatu BMP!");

                    free(bmp_header);
                    free(img_header);
                }
            } else {
                perror("File opening error: ");
            }
            fclose(out_file);
            fclose(in_file);
        }
    } else {
        //printf("%d + %d = %d\n", sizeof(struct BITMAPFILEHEADER), sizeof(struct BITMAPV4HEADER), sizeof(struct BITMAPHEADER));
        puts("Podaj nazwe pliku jako argument!");
    }
    return 0;
}
