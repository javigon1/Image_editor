/*
 *     ppmtrans.c
 *     Javier Gonzalez (jgonza20) and Cheng Li (cli01)
 *     2/23/24
 *     Locality
 *
 *     This program reads a PPM image file, performs image transformations
 *     (rotation, flipping, transposing) based on the commands given, and
 *     writes the transformed image to stdout. Command line is used to specify 
 *     the rotation angle, flip direction, mapping type (row-major,
 *     column-major, block-major), and an optional timing file to record the
 *     execution time and complementary information. The program uses the 
 *     A2Methods interface for the 2D array usage and pnm.h for PPM image 
 *     processing.
 */

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

/* struct to allow us to access the second picture and A2 methods */
struct closure {
        A2Methods_T methods;
        A2Methods_UArray2 array2;
};

/* struct to store information about the image */
struct imageInfo {
        int rotation, width, height;
        char *image_name, *mapping, *transformation;
};

/* function declarations */
void rotate0(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void rotate90(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void rotate180(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void rotate270(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void flipHorizontal(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void flipVertical(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void doTranspose(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void writeTimer(double time_used, char *time_file_name, struct imageInfo);

/* Usage function */
static void usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] "
		        "[-time time_file] "
		        "[filename]\n",
                        progname);
        exit(1);
}


/*
 * Name: main
 * 
 * Description: Main body of the program. Checks command-line arguments to 
 * determine the image transformation operations to be performed, the type of
 * mapping to be used, and the optional timing file name. Reads the input image
 * file, applies the specified transformations using the provided mapping, 
 * records the timing information if specified, and writes the output image to
 * stdout.
 *
 * Parameters:
 *           int argc: the number of command-line arguments
 *           char *argv[]: an array of command-line argument strings
 *        
 * Returns: int. EXIT_SUCCESS if the program completes successfully,
 * non-zero otherwise
 * 
 * Expects: valid command-line arguments and input image file
 * 
 * Notes: Exits with a status code of 1 if there are too many arguments, an 
 * unknown option is provided, or an error occurs while opening or reading the
 * input image file. Uses apply functions (rotate0, rotate90, etc.) to perform 
 * image transformations based on the specified rotation angle, flip direction,
 * or transpose operation. Uses the specified mapping (row-major, column-major,
 * or block-major) to create the new image in the desired format.
 */
int main(int argc, char *argv[])
{
        /* variables that allows us to perform checks throughout main */
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

        /* command line argument parsing */
        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                                    /* to be able to print the mapping if a
                                    time file is given */
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
                                /* to check what flip to do */
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

        /* image file openning */
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

        /* populate orig_image->pixels with the file read */
        Pnm_ppm orig_image = Pnm_ppmread(fp, methods);
        fclose(fp);
        assert(orig_image);

        /* create a new uarray2 to perform the rotation on that one */
        A2Methods_UArray2 new_image;

        /* create and start the timer right before rotating */
        CPUTime_T timer = CPUTime_New();
        CPUTime_Start(timer);

        /* check what transformation to perform */
        if (rotation == 0 && rotation_given) {
                /* set the new image's dimensions */
                new_image = methods->new(methods->width(orig_image), 
                                         methods->height(orig_image),
                                         sizeof(struct Pnm_rgb));
                /* create an instance of closure to access methods and 
                new_image while rotating */
                struct closure infoGet = {methods, new_image};
                /* map with the set major mapping function */
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
        } else if (horizontal) {
                        new_image = methods->new(methods->width(orig_image), 
                                                 methods->height(orig_image),
                                                 sizeof(struct Pnm_rgb));
                        struct closure infoGet = {methods, new_image};
                        map(orig_image->pixels, flipHorizontal, &infoGet);
        } else if (vertical) {
                        new_image = methods->new(methods->width(orig_image), 
                                                 methods->height(orig_image),
                                                 sizeof(struct Pnm_rgb));
                        struct closure infoGet = {methods, new_image};
                        map(orig_image->pixels, flipVertical, &infoGet);
        } else if (transpose) {
                        new_image = methods->new(methods->height(orig_image), 
                                                 methods->width(orig_image),
                                                 sizeof(struct Pnm_rgb));
                        struct closure infoGet = {methods, new_image};
                        map(orig_image->pixels, doTranspose, &infoGet);
        }

        /* stop the timer right after the rotation has been performed */
        double time_used = CPUTime_Stop(timer);

        /* Check if a time file has been given, if so, print the information
        gathered to the time file (appending it)*/
        if (time_file_name != NULL) {
                char *transformation;
                /* check if a non-rotation transformation was given */
                if (vertical) {
                        transformation = "vertical";
                } else if (horizontal) {
                        transformation = "horizontal";
                } else if (transpose) {
                        transformation = "transpose";
                } else {
                        transformation = "NO";
                }
                        
                struct imageInfo image_info = { rotation, 
                                                methods->width(orig_image),
                                                methods->height(orig_image),
                                                argv[argc - 1], 
                                                mapping, transformation };
                writeTimer(time_used, time_file_name, image_info);
        }

        /* free the information */
        CPUTime_Free(&timer);
        methods->free(&orig_image->pixels);
        orig_image->width = methods->width(new_image);
        orig_image->height = methods->height(new_image);
        orig_image->pixels = new_image;

        /* write the transformed image to standard output */
        Pnm_ppmwrite(stdout, orig_image);
        Pnm_ppmfree(&orig_image);

        return EXIT_SUCCESS;
}

/*
 * Name: rotate0
 * 
 * Description: rotates the given UArray2 by 0 degrees. This is called in main 
 * as an apply function, which means that it maps each element it is called on
 * (each pixel) to the respective position in the rotated image
 *
 * Parameters:
 *           int i: the column position the elem is at
 *           int j: the row position the elem is at
 *           A2Methods_UArray2 array2: the UArray containing the image
 *           void *elem: the element at the previously mentioned location in the
 *           uarray containing the image
 *           void *cl: the struct containing the methods and the second UArray2
 *           we need to create the new image (rotated version)
 *        
 * Returns: nothing
 * 
 * Expects: valid position of the pixel
 * 
 * Notes: results in checked runtime error if: the column pos is a non-positive 
 * value or the row is a non-positive value 
 */
void rotate0(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        /* check for the bounds passed as parameters */
        assert(i >= 0 && j >= 0);
        (void)array2;
        /* create instance of the struct to be able to access its information */
        struct closure *info = cl;
        int height = info->methods->height(array2);
        int width = info->methods->width(array2);

        /* check for the mapping coordinates to be inside of bounds */
        if (i < 0 || i >= width || j < 0 || j >= height) {
                fprintf(stderr, 
                        "Error: Invalid pixel coordinates (%d, %d)->(%d, %d)\n",
                        i, j, height - j - 1, i);
                exit(EXIT_FAILURE);
        }

        /* element in the original array*/
        Pnm_rgb array_pixel = elem;
        /* position where we want to store the element in the tranformed 
        image */
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, i, j);
        /* assign the element read to the respective position in the transformed
        image */
        *rotated_pixel = *array_pixel;
}       


/*
 * Name: rotate90
 * 
 * Description: rotates the given UArray2 by 90 degrees. This is called in main 
 * as an apply function, which means that it maps each element it is called on
 * (each pixel) to the respective position in the rotated image
 *
 * Parameters:
 *           int i: the column position the elem is at
 *           int j: the row position the elem is at
 *           A2Methods_UArray2 array2: the UArray containing the image
 *           void *elem: the element at the previously mentioned location in the
 *           uarray containing the image
 *           void *cl: the struct containing the methods and the second UArray2
 *           we need to create the new image (rotated version)
 *        
 * Returns: nothing
 * 
 * Expects: valid position of the pixel
 * 
 * Notes: results in checked runtime error if: the column pos is a non-positive 
 * value or the row is a non-positive value 
 */
void rotate90(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        assert(i >= 0 && j >= 0);
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


/*
 * Name: rotate180
 * 
 * Description: rotates the given UArray2 by 180 degrees. This is called in main 
 * as an apply function, which means that it maps each element it is called on
 * (each pixel) to the respective position in the rotated image
 *
 * Parameters:
 *           int i: the column position the elem is at
 *           int j: the row position the elem is at
 *           A2Methods_UArray2 array2: the UArray containing the image
 *           void *elem: the element at the previously mentioned location in the
 *           uarray containing the image
 *           void *cl: the struct containing the methods and the second UArray2
 *           we need to create the new image (rotated version)
 *        
 * Returns: nothing
 * 
 * Expects: valid position of the pixel
 * 
 * Notes: results in checked runtime error if: the column pos is a non-positive 
 * value or the row is a non-positive value 
 */
void rotate180(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        assert(i >= 0 && j >= 0);
        struct closure *info = cl;
        int height = info->methods->height(array2);
        int width = info->methods->width(array2);

        if (i < 0 || i >= width || j < 0 || j >= height) {
                fprintf(stderr, 
                "Error: Invalid pixel coordinates (%d, %d)\n",
                i, j);
        exit(EXIT_FAILURE);
        }

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, width - i - 1, 
                                                  height - j - 1);
        *rotated_pixel = *array_pixel;
}


/*
 * Name: rotate270
 * 
 * Description: rotates the given UArray2 by 270 degrees. This is called in main 
 * as an apply function, which means that it maps each element it is called on
 * (each pixel) to the respective position in the rotated image
 *
 * Parameters:
 *           int i: the column position the elem is at
 *           int j: the row position the elem is at
 *           A2Methods_UArray2 array2: the UArray containing the image
 *           void *elem: the element at the previously mentioned location in the
 *           uarray containing the image
 *           void *cl: the struct containing the methods and the second UArray2
 *           we need to create the new image (rotated version)
 *        
 * Returns: nothing
 * 
 * Expects: valid position of the pixel
 * 
 * Notes: results in checked runtime error if: the column pos is a non-positive 
 * value or the row is a non-positive value 
 */
void rotate270(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        assert(i >= 0 && j >= 0);
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


/*
 * Name: flipHorizontal
 * 
 * Description: flips the given UArray2 horizontally. This is called in main 
 * as an apply function, which means that it maps each element it is called on
 * (each pixel) to the respective position in the flipped image
 *
 * Parameters:
 *           int i: the column position the elem is at
 *           int j: the row position the elem is at
 *           A2Methods_UArray2 array2: the UArray containing the image
 *           void *elem: the element at the previously mentioned location in the
 *           uarray containing the image
 *           void *cl: the struct containing the methods and the second UArray2
 *           we need to create the new image (flipped version)
 *        
 * Returns: nothing
 * 
 * Expects: valid position of the pixel
 * 
 * Notes: results in checked runtime error if: the column pos is a non-positive 
 * value or the row is a non-positive value 
 */
void flipHorizontal(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        assert(i >= 0 && j >= 0);
        struct closure *info = cl;
        int width = info->methods->width(array2);

        if (width - i - 1 < 0 || width - i - 1 >= width || 
            j < 0 || j >= width) {
                fprintf(stderr, 
                        "Error: Invalid pixel coordinates (%d, %d)->(%d, %d)\n",
                        i, j, width - i - 1, j);
                exit(EXIT_FAILURE);
        }

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, width - i - 1, j);
        *rotated_pixel = *array_pixel;
}


/*
 * Name: flipVertical
 * 
 * Description: flips the given UArray2 vertically. This is called in main 
 * as an apply function, which means that it maps each element it is called on
 * (each pixel) to the respective position in the flipped image
 *
 * Parameters:
 *           int i: the column position the elem is at
 *           int j: the row position the elem is at
 *           A2Methods_UArray2 array2: the UArray containing the image
 *           void *elem: the element at the previously mentioned location in the
 *           uarray containing the image
 *           void *cl: the struct containing the methods and the second UArray2
 *           we need to create the new image (flipped version)
 *        
 * Returns: nothing
 * 
 * Expects: valid position of the pixel
 * 
 * Notes: results in checked runtime error if: the column pos is a non-positive 
 * value or the row is a non-positive value 
 */
void flipVertical(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        assert(i >= 0 && j >= 0);
        struct closure *info = cl;
        int height = info->methods->height(array2);

        // if (i < 0 || i >= height || height - j - 1 < 0 || 
        //                             height - j - 1 >= height) {
        //         fprintf(stderr, 
        //                 "Error: Invalid pixel coordinates (%d, %d)->(%d, %d)\n",
        //                 i, j, i, height - j - 1);
        //         exit(EXIT_FAILURE);
        // }

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, i, height - j - 1);
        *rotated_pixel = *array_pixel;
}


/*
 * Name: doTranspose
 * 
 * Description: transposes the given UArray2. This is called in main 
 * as an apply function, which means that it maps each element it is called on
 * (each pixel) to the respective position in the transposed image
 *
 * Parameters:
 *           int i: the column position the elem is at
 *           int j: the row position the elem is at
 *           A2Methods_UArray2 array2: the UArray containing the image
 *           void *elem: the element at the previously mentioned location in the
 *           uarray containing the image
 *           void *cl: the struct containing the methods and the second UArray2
 *           we need to create the new image (transposed version)
 *        
 * Returns: nothing
 * 
 * Expects: valid position of the pixel
 * 
 * Notes: results in checked runtime error if: the column pos is a non-positive 
 * value or the row is a non-positive value 
 */
void doTranspose(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl) 
{
        assert(i >= 0 && j >= 0);
        struct closure *info = cl;
        int height = info->methods->height(array2);
        int width = info->methods->width(array2);

        if (i < 0 || i >= width || j < 0 || j >= height) {
                fprintf(stderr, 
                        "Error: Invalid pixel coordinates (%d, %d)\n",
                        i, j);
                exit(EXIT_FAILURE);
        }

        Pnm_rgb array_pixel = elem;
        Pnm_rgb rotated_pixel = info->methods->at(info->array2, j, i);
        *rotated_pixel = *array_pixel;
}


/*
 * Name: writeTimer
 * 
 * Description: writes timing information to a file, including details about
 * the image, rotation, transformation, mapping, time taken, and time per pixel
 *
 * Parameters:
 *           double time_used: the total time taken for the operation
 *           char *time_file_name: the name of the file to write the timing information to
 *           struct imageInfo image_info: a struct containing information about the image
 *        
 * Returns: nothing
 * 
 * Expects: valid file name and image information
 * 
 * Notes: results in a checked runtime error if the file cannot be opened for writing
 */
void writeTimer(double time_used, char *time_file_name, 
                struct imageInfo image_info)
{
        /* open the output file in append mode - to append the information */
        FILE *time_file = fopen(time_file_name, "a");

        /* check for a given file */
        if (time_file == NULL) {
                fprintf(stderr, "Error opening file: %s\n", time_file_name);
                return;
        }

        /* get the total number of pixels to be able to calculate the time per 
        pixel */
        int pixel_total = image_info.width * image_info.height;
        double time_per_pixel = time_used / pixel_total;

        /* print the information */
        fprintf(time_file, "FILE: %s\n"
                "Image has a width of %d pixels and a height of %d pixels\n"
                "Rotation: %d degree\n"
                "Transformation: %s\n"
                "Mapping: %s \n"
                "Time taken: %0.f nanoseconds\n"
                "Time taken on each pixel: %0.f nanoseconds\n", 
                image_info.image_name, 
                image_info.width, 
                image_info.height, 
                image_info.rotation, 
                image_info.transformation, 
                image_info.mapping, 
                time_used, 
                time_per_pixel);
        
        fclose(time_file);
}
