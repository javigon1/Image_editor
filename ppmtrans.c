#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"

#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (false)


void rotate90(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void rotate180(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void rotate0(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);

static void usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] "
		        "[-time time_file] "
		        "[filename]\n",
                        progname);
        exit(1);
}

struct closure {
        A2Methods_T methods;
        A2Methods_UArray2 array2;
};


int main(int argc, char *argv[])
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;
        bool file_given   = false;

        (void)time_file_name;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods != NULL);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-time") == 0) {
                        if (!(i + 1 < argc)) {      /* no time file */
                                usage(argv[0]);
                        }
                        time_file_name = argv[++i];
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        file_given = true;
                }
        }

        FILE *fp;
        if (!file_given) {
                fp = stdin;
        } else {
                fp = fopen(argv[argc - 1], "rb");
                if (!fp) {
                        fprintf(stderr, 
                                "Error: Cannot open file '%s' for reading.\n", 
                                argv[argc - 1]);
                        exit(EXIT_FAILURE);
                }
        }

        Pnm_ppm orig_image = Pnm_ppmread(fp, methods);
        fclose(fp);
        assert(orig_image);

        A2Methods_UArray2 new_image;
        // fprintf(stderr, "Width at the start: %d\n", methods->width(orig_image));
        // fprintf(stderr, "Height at the start: %d\n", methods->height(orig_image));

        if (rotation == 0) {
                new_image = methods->new(methods->width(orig_image), 
                                         methods->height(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, rotate90, &infoGet);
        } else if (rotation == 90) {
                // fprintf(stderr, "Width: %d\n", methods->height(orig_image));
                // fprintf(stderr, "Height: %d\n", methods->width(orig_image));
                new_image = methods->new(methods->height(orig_image), 
                                         methods->width(orig_image),
                                         sizeof(struct Pnm_rgb));
                                        // fprintf(stderr, "djhi\n");
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, rotate90, &infoGet);
                // SWAP THE VALUES OF THE WIDTH AND THE HEIGHT
        } else if (rotation == 180) {
                new_image = methods->new(methods->width(orig_image), 
                                         methods->height(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, rotate90, &infoGet);
        } else if (rotation == 270) {
                // bonus over here
        } else {
                // other bonuses
        }

        methods->free(&orig_image->pixels);
        orig_image->width = methods->width(new_image);
        orig_image->height = methods->height(new_image);
        orig_image->pixels = new_image;

        Pnm_ppmwrite(stdout, orig_image);
        Pnm_ppmfree(&orig_image);

        return 0;
}


void rotate90(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        // (void)array2;
        // fprintf(stderr, "hdbjdns\n");
        struct closure *info = cl;
        
        int height = info->methods->height(array2);
        int width = info->methods->width(array2);

        // fprintf(stderr, "Rotating pixel [%d, %d] -> [%d, %d]\n", i, j, height - j - 1, i);

        if (height - j - 1 < 0 || height - j - 1 >= height || i < 0 || i >= width) {
                fprintf(stderr, "Error: Invalid pixel coordinates (%d, %d) -> (%d, %d)\n", i, j, height - j - 1, i);
                exit(EXIT_FAILURE);
        }

        // fprintf(stderr, "Accessing pixel at [%d, %d]\n", height - j - 1, i);
        Pnm_rgb array_pixel = elem;
        // fprintf(stderr, "Got element\n");
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, height - j - 1, i);
        // fprintf(stderr, "Got rotated pixel\n");
        *rotated_pixel = *array_pixel;
        // fprintf(stderr, "Performed conversion\n");
}


void rotate180(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        (void)array2;
        Pnm_ppm image = cl;
        // Pnm_rgb array_pixel = elem;

        int height = image->height;
        int width = image->width;
        Pnm_rgb rotated_pixel = image->methods->at(image->pixels, width - i - 1, height - j - 1);
        *((Pnm_rgb)elem) = *rotated_pixel;
}


void rotate0(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        (void)array2;
        Pnm_ppm image = cl;
        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = image->methods->at(image->pixels, i, j);
        *array_pixel = *rotated_pixel;
}       