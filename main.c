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
    unsigned short bfType;
    unsigned long bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long bfOffBits;
};

struct BITMAPINFOHEADER {
    unsigned long biSize;
    long biWidth;
    long biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long biCompression;
    unsigned long biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    unsigned long biClrUsed;
    unsigned long biClrImportant;
};

struct BITMAPHEADER {
    struct BITMAPFILEHEADER bf;
    struct BITMAPINFOHEADER bi;
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

    bmp_header->bf.bfType = 0x424D;
    bmp_header->bf.bfOffBits = sizeof(struct BITMAPFILEHEADER) + sizeof(struct BITMAPINFOHEADER);

    bmp_header->bi.biSize = sizeof(struct BITMAPINFOHEADER);
    bmp_header->bi.biWidth = img_header->ihWidth;
    bmp_header->bi.biHeight = img_header->ihHeight;
    bmp_header->bi.biPlanes = 1;
    bmp_header->bi.biBitCount = img_header->ihBitCount;
    bmp_header->bi.biCompression = img_header->ihCompresion;
    bmp_header->bi.biSizeImage = img_header->ihSizeImage;
    bmp_header->bi.biXPelsPerMeter = 2835;
    bmp_header->bi.biYPelsPerMeter = 2835;
    bmp_header->bi.biClrUsed = 0;
    bmp_header->bi.biClrImportant = 0;

    bmp_header->bf.bfSize = bmp_header->bf.bfOffBits + bmp_header->bi.biSizeImage;

    return bmp_header;
}

void write_bmp(FILE *bmp_file, struct BITMAPHEADER *bmp_header, FILE *img_file)
{
    fpos_t init_pos;
    fgetpos(img_file, &init_pos);
    fseek(img_file, 0, SEEK_SET);

    fwrite(bmp_header, 1, sizeof(struct BITMAPHEADER), bmp_file);
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
        FILE *in_file = fopen(argv[1], "rb"), *out_file = fopen("out.bmp", "wb");
        struct IMGHEADER *img_header = read_img_header(in_file);
        if (img_header != NULL) {
            struct BITMAPHEADER *bmp_header = prepare_bmp_header(img_header);

            puts("Wczytano naglowek!");
            printf("Rozmiar obrazu: %d x %d px\n", img_header->ihWidth, abs(img_header->ihHeight));

            write_bmp(out_file, bmp_header, in_file);

            free(bmp_header);
            free(img_header);
        }
    } else {
        puts("Podaj nazwe pliku jako argument!");
    }
    return 0;
}
