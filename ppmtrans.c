#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "cputiming.h"
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


struct closure {
        A2Methods_T methods;
        A2Methods_UArray2 array2;
};

struct imageInfo {
        int rotation, width, height;
        char *image_name, *mapping, *transformation;
};


void rotate0(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void rotate90(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void rotate180(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void rotate270(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void flipHorizontal(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void flipVertical(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void doTranspose(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void writeTimer(double time_used, char *time_file_name, struct imageInfo);

static void usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] "
		        "[-time time_file] "
		        "[filename]\n",
                        progname);
        exit(1);
}


int main(int argc, char *argv[])
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;
        bool  file_given     = false;
        bool  horizontal     = false;
        bool  vertical       = false;
        bool  transpose      = false;
        char  *mapping       = "row-major";
        bool  rotation_given = false;

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
                                    mapping = "row major";
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                                    mapping = "column major";
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                                    mapping = "block major";
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
                        if (rotation == 0) {
                                rotation_given = true;
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-flip") == 0) {
                        if (!(i + 1 < argc)) {
                                fprintf(stderr, "Direction of flip required\n");
                                exit(1);
                        }
                        i++;
                        if (strcmp(argv[i], "horizontal") == 0) {
                                horizontal = true;
                        } else if (strcmp(argv[i], "vertical") == 0) {
                                vertical = true;
                        } else {
                                fprintf(stderr, "Invalid direction of flip\n");
                                exit(1);
                        }
                } else if (strcmp(argv[i], "-transpose") == 0) {
                        transpose = true;
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
        // queries populated after that: rotation, mapping major, time_file
        
        // open the file to read
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

        (void)time_file_name;

        // ppm variable to take over the image info read from file.
        Pnm_ppm orig_image = Pnm_ppmread(fp, methods);
        fclose(fp);
        assert(orig_image);

        A2Methods_UArray2 new_image;

        // declarations used for counting time
        CPUTime_T timer = CPUTime_New();

        // only timing rotations (operations)
        CPUTime_Start(timer);

        if (rotation == 0 && rotation_given) {
                new_image = methods->new(methods->width(orig_image), 
                                         methods->height(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, rotate0, &infoGet);
        } else if (rotation == 90) {
                new_image = methods->new(methods->height(orig_image), 
                                         methods->width(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, rotate90, &infoGet);
        } else if (rotation == 180) {
                new_image = methods->new(methods->width(orig_image), 
                                         methods->height(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, rotate180, &infoGet);
        } else if (rotation == 270) {
                new_image = methods->new(methods->height(orig_image), 
                                         methods->width(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, rotate270, &infoGet);
        }
        
        if (horizontal) {
                new_image = methods->new(methods->width(orig_image), 
                                         methods->height(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, flipHorizontal, &infoGet);
        } 

        if (vertical) {
                new_image = methods->new(methods->width(orig_image), 
                                         methods->height(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, flipVertical, &infoGet);
        } 
        
        if (transpose) {
                new_image = methods->new(methods->height(orig_image), 
                                         methods->width(orig_image),
                                         sizeof(struct Pnm_rgb));
                struct closure infoGet = {methods, new_image};
                map(orig_image->pixels, doTranspose, &infoGet);
        }

        double time_used = CPUTime_Stop(timer);

        if (time_file_name != NULL) {
                char *transformation = (horizontal) ? "horizontal" : "NO" ;
                transformation = (vertical) ? "vertical" : "NO" ;
                transformation = (transpose) ? "transpose" : "NO" ;
                struct imageInfo image_info = { rotation, 
                                                methods->width(orig_image),
                                                methods->height(orig_image),
                                                argv[argc - 1], 
                                                mapping, transformation };
                writeTimer(time_used, time_file_name, image_info);
        }
        

        CPUTime_Free(&timer);

        methods->free(&orig_image->pixels);
        orig_image->width = methods->width(new_image);
        orig_image->height = methods->height(new_image);
        orig_image->pixels = new_image;

        Pnm_ppmwrite(stdout, orig_image);
        Pnm_ppmfree(&orig_image);

        return 0;
}

/* apply functions start here */
/*
 * rotate0
 * parameter: int i, the column of the current element is applying
 *            int j, the row of the current element
 *            a (polymorphoic) UArray2 / UArray2b 
 *            void, the current element (under the hood was pointed through
 *                                       At in uarray2b.c or uarray2.c)
 *            void, the closure
 * return:    N/A
 * 
 */
void rotate0(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        (void)array2;
        struct closure *info = cl;

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, i, j);
        *rotated_pixel = *array_pixel;
}       

void rotate90(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        struct closure *info = cl;
        int height = info->methods->height(array2);
        int width = info->methods->width(array2);

        if (height - j - 1 < 0 || height - j - 1 >= height || 
                                  i < 0 || i >= width) {
                fprintf(stderr, 
                        "Error: Invalid pixel coordinates (%d, %d)->(%d, %d)\n",
                        i, j, height - j - 1, i);
                exit(EXIT_FAILURE);
        }

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, 
                                                  height - j - 1, i);
        *rotated_pixel = *array_pixel;
}


void rotate180(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        struct closure *info = cl;
        int height = info->methods->height(array2);
        int width = info->methods->width(array2);

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, width - i - 1, 
                                                  height - j - 1);
        *rotated_pixel = *array_pixel;
}


void rotate270(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        struct closure *info = cl;
        int height = info->methods->height(array2);
        int width = info->methods->width(array2);

        if (j < 0 || j >= height || width - i - 1 < 0 || 
                                    width - i - 1 >= width) {
                fprintf(stderr, 
                        "Error: Invalid pixel coordinates (%d, %d)->(%d, %d)\n",
                        i, j, height - j - 1, i);
                exit(EXIT_FAILURE);
        }

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, j, width - i - 1);
        *rotated_pixel = *array_pixel;
}  


void flipHorizontal(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        struct closure *info = cl;
        int width = info->methods->width(array2);

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, width - i - 1, j);
        *rotated_pixel = *array_pixel;
}


void flipVertical(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        struct closure *info = cl;
        int height = info->methods->height(array2);

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, i, height - j - 1);
        *rotated_pixel = *array_pixel;
}


void doTranspose(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl) 
{
        (void)array2;
        struct closure *info = cl;

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, j, i);
        *rotated_pixel = *array_pixel;
}

/* apply functions end */

/* write time info to a file */
void writeTimer(double time_used, char *time_file_name, 
                struct imageInfo image_info)
{
        FILE *time_file = fopen(time_file_name, "a"); // opened in append mode

        if (time_file == NULL) {
                fprintf(stderr, "Error opening file: %s\n", time_file_name);
                return;
        }

        int pixel_total = image_info.width * image_info.height;
        double time_per_pixel = time_used / pixel_total;

        // Write the timing information to the file
        // may improve by including more image info
        fprintf(time_file, "FILE: %s\nImage contains a width of %d pixels and a"
                " height of %d pixels\nRotation: %d degree\nTransformation: %s\n"
                "Mapping: %s \nTime taken: %0.f nanoseconds\nTime taken on each"
                " pixel: %0.f nanoseconds\n", image_info.image_name, 
                image_info.width, image_info.height, image_info.rotation, 
                image_info.transformation, image_info.mapping, time_used, 
                time_per_pixel);

        // Close the file
        fclose(time_file);
}