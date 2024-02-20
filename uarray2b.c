#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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

T UArray2b_new (int width, int height, int size, int blocksize) 
{

}


T UArray2b_new_64K_block(int width, int height, int size)
{

}


void UArray2b_free (T *array2b)
{

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
        return UArray_at(row(array2b, row), column);
}


void UArray2b_map(T array2b, void apply(int col, int row, T array2b, void *elem,
                  void *cl), void *cl)
{

}