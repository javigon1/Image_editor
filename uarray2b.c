#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mem.h"
#include "uarray2b.h"
#include "uarray2.h"
#include "uarray.h"

#define T UArray2b_T

struct T {
    int width;
    int height;
    int size;
    int blocksize;
    UArray2_T blocks;
};


// static int is_ok(T a)
// {
//         return a && UArray2_height(a->blocks) == a->height &&
//                 UArray2_width(a->blocks) == ((a->width + a->blocksize - 1) / a->blocksize) &&
//                 UArray2_size(a->blocks) == sizeof(UArray2_T) &&
//                 (a->height == 0 || 
//                 (UArray2_width(UArray2_at(a->blocks, 0, 0)) == ((a->width + a->blocksize - 1) / a->blocksize) &&
//                 UArray2_size(UArray2_at(a->blocks, 0, 0)) == a->size));
// }


static inline int num_blocks(int dimension, int blocksize) {
    return (dimension + blocksize - 1) / blocksize;
}


T UArray2b_new (int width, int height, int size, int blocksize) 
{
        T array2b;
        NEW(array2b);
        array2b->width  = width;
        array2b->height = height;
        array2b->size   = size;

        int block_width = (width + blocksize - 1) / blocksize;
        int block_height = (height + blocksize - 1) / blocksize;

        array2b->blocks = UArray2_new(block_width, block_height, sizeof(UArray_T));
        assert(array2b->blocks != NULL);

        for (int i = 0; i < block_width; i++) {
                for (int j = 0; j < block_height; j++) {
                        UArray_T *block = UArray2_at(array2b->blocks, i, j);
                        *block = UArray_new(blocksize * blocksize, size);
                }
        }

        return array2b;
}


T UArray2b_new_64K_block(int width, int height, int size)
{
        int blocksize = 1;
        while (blocksize * blocksize * size <= 64 * 1024) {
                blocksize++;
        }
        blocksize--;

        return UArray2b_new(width, height, size, blocksize);
}


void UArray2b_free (T *array2b)
{
        assert(array2b != NULL && *array2b != NULL);
        
        int block_width = ( (*array2b)->width + (*array2b)->blocksize - 1) / (*array2b)->blocksize;
        int block_height = ( (*array2b)->height + (*array2b)->blocksize - 1) / (*array2b)->blocksize;

        for (int i = 0; i < block_width; i++) {
                for (int j = 0; j < block_height; j++) {
                        UArray_T *block = UArray2_at((*array2b)->blocks, i, j);
                        UArray_free(block);
                }
        }

        UArray2_free(&((*array2b)->blocks));
        FREE(*array2b);
}

int UArray2b_width (T array2b)
{
        assert(array2b != NULL);
        return array2b->width;
}


int UArray2b_height (T array2b)
{
        assert(array2b != NULL);
        return array2b->height;
}


int UArray2b_size (T array2b)
{
        assert(array2b != NULL);
        return array2b->size;
}


int UArray2b_blocksize(T array2b)
{
        assert(array2b != NULL);
        return array2b->blocksize;
}


void *UArray2b_at(T array2b, int column, int row)
{
        assert(array2b != NULL);
        assert(column >= 0 && column < array2b->width && row > 0 && row <= array2b->height);

        int blocksize = array2b->blocksize;

        int block_col = column / blocksize;
        int block_row = row / blocksize;

        int index = (row % blocksize) * blocksize + (column % blocksize);

        UArray_T *block = UArray2_at(array2b->blocks, block_col, block_row);
        return UArray_at(*block, index);
}


void UArray2b_map(T array2b, void apply(int col, int row, T array2b, void *elem,
                  void *cl), void *cl)
{
        assert(array2b != NULL);

        int blocksize = array2b->blocksize;
        int width = array2b->width;
        int height = array2b->height;

        for (int block_row = 0; block_row < height; block_row += blocksize) {
                for (int block_col = 0; block_col < width; block_col += blocksize) {
                        for (int row = block_row; row < block_row + blocksize && row < height; row++) {
                                for (int col = block_col; col < block_col + blocksize && col < width; col++) {
                                        apply(col, row, array2b, UArray2b_at(array2b, col, row), cl);
                                }
                        }
                }
        }
}
