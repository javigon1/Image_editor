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
        int count            = 0;
        bool file_given   = false;

        (void)time_file_name;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_blocked; 
        assert(methods != NULL);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                        count++;
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                        count++;
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                        count++;
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        count++;
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
                        count++;
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
                        break;
                }
        }

        /**********************cli 2/21****************************/
        FILE *fp;
        if (!file_given) {
                printf("no file given!\n");
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

        /* DIFFERENT USES!
        ./ppmtrans -rotate 90 -col-major -time filename 
        ./ppmtrans -rotate 0 filename
        ./ppmtrans -rotate 90 -time filename
        .ppmtrans - rotate 0 -col-major filename */

        Pnm_ppm orig_image = Pnm_ppmread(fp, methods);
        assert(orig_image);

        fclose(fp);

        Pnm_ppmwrite(stdout, orig_image);

        Pnm_ppmfree(&orig_image);

        /**********************cli 2/21****************************/

        return 0;
}
