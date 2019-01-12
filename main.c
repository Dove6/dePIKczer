#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int from_LE(unsigned const char *LE)
{
    return (LE[0] | (LE[1] << 8) | (LE[2] << 16) | (LE[3] << 24));
}

struct IMGHEADER {
    int width;
    int height;
    int bpp;
    int data_size;
    int something;
    int version;
    int something2;
    int pos_x;
    int pos_y;
};

struct IMGHEADER *read_img(FILE *img_file)
{
    fpos_t init_pos;
    fgetpos(img_file, &init_pos);
    fseek(img_file, 0, SEEK_SET);

    char id_buff[4];
    fread(id_buff, 1, 4, img_file);
    if (!strcmp(id_buff, "PIK")) {
        puts("Wooo");
        struct IMGHEADER *img_header = malloc(sizeof(struct IMGHEADER));
        fread(img_header, 1, sizeof(struct IMGHEADER), img_file);
        return img_header;
    } else {
        puts("Nieprawidlowy format pliku!");
    }

    fsetpos(img_file, &init_pos);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        FILE *in_file = fopen(argv[1], "rb"), *out_file = fopen("out.bmp", "wb");
        struct IMGHEADER *img_header = read_img(in_file);

        puts("Wczytano naglowek!");
        printf("Rozmiar obrazu: %d x %d px\n", img_header->width, abs(img_header->height));

        free(img_header);
    }
    return 0;
}
