#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include "despeckle.h"

void scalenx_usage(char* prog, int radius, float coef)
{
    printf("Stb Image Despeckle version %s.\n", DESPECKLE_VERSION);
    printf("usage: %s [options] image_in image_out.png\n", prog);
    printf("options:\n");
    printf("  -c N.N    coef (default %f)\n", coef);
    printf("  -r NUM    radius (default %d)\n", radius);
    printf("  -h        show this help message and exit\n");
}

int main(int argc, char **argv)
{
    int height, width, channels, y, x, d;
    int resize_height = 0, resize_width = 0;
    float coef = 0.25f;
    int radius = 5;
    int fhelp = 0;
    int opt;
    size_t ki, kd;
    unsigned char *data = NULL, *out_data = NULL;
    stbi_uc *img = NULL;

    while ((opt = getopt(argc, argv, ":c:r:h")) != -1)
    {
        switch(opt)
        {
        case 'c':
            coef = atof(optarg);
            break;
        case 'r':
            radius = atoi(optarg);
            if (radius < 1)
            {
                fprintf(stderr, "ERROR: radius < 1: %d\n", radius);
                return 1;
            }
            break;
         case 'h':
            fhelp = 1;
            break;
        case ':':
            fprintf(stderr, "ERROR: option needs a value\n");
            return 2;
            break;
        case '?':
            fprintf(stderr, "ERROR: unknown option: %c\n", optopt);
            return 3;
            break;
        }
    }
    if(optind + 2 > argc || fhelp)
    {
        scalenx_usage(argv[0], radius, coef);
        return 0;
    }
    const char *src_name = argv[optind];
    const char *dst_name = argv[optind + 1];


    printf("Load: %s\n", src_name);
    if (!(img = stbi_load(src_name, &width, &height, &channels, STBI_rgb_alpha)))
    {
        fprintf(stderr, "ERROR: not read image: %s\n", src_name);
        return 1;
    }
    printf("image: %dx%d:%d\n", width, height, channels);
    if (!(data = (unsigned char*)malloc(height * width * channels * sizeof(unsigned char))))
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 1;
    }
    ki = 0;
    kd = 0;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            for (d = 0; d < channels; d++)
            {
                data[kd + d] = (unsigned char)img[ki + d];
            }
            ki += STBI_rgb_alpha;
            kd += channels;
        }
    }
    stbi_image_free(img);

    if (!(out_data = (unsigned char*)malloc(height * width * channels * sizeof(unsigned char))))
    {
        fprintf(stderr, "ERROR: not use memmory\n");
        return 2;
    }

    printf("Despeckle %d %f\n", radius, coef);
    Despeckle (data, height, width, channels, radius, coef, out_data);

    printf("Save png: %s\n", dst_name);
    if (!(stbi_write_png(dst_name, width, height, channels, out_data, width * channels)))
    {
        fprintf(stderr, "ERROR: not write image: %s\n", dst_name);
        return 1;
    }

    free(out_data);
    free(data);

    return 0;
}
