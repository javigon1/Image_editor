/*
 *     uarray2b.c
 *     Javier Gonzalez (jgonza20) and Cheng Li (cli01)
 *     2/23/24
 *     Locality
 *
 *     uarray2b represents a two dimensional array that can store elements in 
 *     its slots. The way this is done is by dividing the array2b into blocks
 *     so that we can then perfom the mapping in block-major mapping, which 
 *     allowing simulates how by block major mapping we can feed whole chunks
 *     of contigous information to the cache. We provide ways to change, access, 
 *     and view what is being stored in the uarray2.
 *    
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mem.h"
#include "uarray2b.h"
#include "uarray2.h"
#include <uarray.h>


#define T UArray2b_T

/* Struct definition for UArray2b */
struct T {
    int width;
    int height;
    int size;
    int blocksize;
    /* each block is represented by a uarray */
    UArray2_T blocks;
};


/*
 * Name: UArray2b_new
 * 
 * Description: Creates a new UArray2b structure with the specified width, height,
 * size, and blocksize. Allocates memory for the blocks array and initializes each
 * block with a new UArray_T.
 *
 * Parameters:
 *           int width: the width of the UArray2b
 *           int height: the height of the UArray2b
 *           int size: the size of each element in the UArray2b
 *           int blocksize: the size of each block in the UArray2b
 *        
 * Returns: the UArray2b_T we created 
 * 
 * Expects: valid dimensions to create a correc UArray2b
 * 
 * Notes: results in checked runtime errors for any of the dimensions being 
 * non positive or 0
 */
T UArray2b_new (int width, int height, int size, int blocksize) 
{
        /* check for correct dimensions given */
        assert(blocksize > 0);
        assert(width > 0);
        assert(height > 0);
        assert(size > 0);

        /* create the instance of the blocked 2D array */
        T array2b;
        NEW(array2b);
        array2b->width     = width;
        array2b->height    = height;
        array2b->size      = size;
        array2b->blocksize = blocksize;

        /* get the corresponding dimensions of each block */
        int block_width  = (width + blocksize - 1) / blocksize;
        int block_height = (height + blocksize - 1) / blocksize;

        /* create a UArray2 of blocks */
        array2b->blocks = UArray2_new(block_width, block_height, sizeof(UArray_T));
        assert(array2b->blocks != NULL);

        /* for each element in the previous UArray2 create a UArray that 
        represents a block */
        for (int i = 0; i < block_width; i++) {
                for (int j = 0; j < block_height; j++) {
                        UArray_T *block = UArray2_at(array2b->blocks, i, j);
                        /* create the uarray */
                        *block = UArray_new(blocksize * blocksize, size);
                }
        }
        return array2b;
}


/*
 * Name: UArray2b_new_64K_block
 * 
 * Description: Creates a new UArray2b structure with the specified width, height,
 * and size, using a blocksize that maximizes the block size to fit within a 64K
 * memory limit.
 *
 * Parameters:
 *           int width: the width of the UArray2b
 *           int height: the height of the UArray2b
 *           int size: the size of each element in the UArray2b
 *        
 * Returns: the UArray2b_T created
 * 
 * Expects: valid dimensions to correclty create the UArray2b
 * 
 * Notes: will result in checked runtime error if any of the dimensions are
 * non-positive or 0
 */
T UArray2b_new_64K_block(int width, int height, int size)
{
        assert(width > 0);
        assert(height > 0);
        assert(size > 0);

        /* default blocksize */
        int blocksize = 1;
        /* store a block as large as possible (MAX 64kb)*/
        while (blocksize * blocksize * size <= 64 * 1024) {
                blocksize++;
        }
        /* we are reading one extra so remove one*/
        blocksize--;

        return UArray2b_new(width, height, size, blocksize);
}


/*
 * Name: UArray2b_free
 * 
 * Description: Frees the memory used by the UArray2b strutc and its blocks.
 *
 * Parameters:
 *           T *array2b: a pointer to the UArray2b struct
 *        
 * Returns: None
 * 
 * Expects: array2b points to a valid UArray2b struct that was previously 
 *          allocated with UArray2b_new and has not been freed before
 * 
 * Notes: results in a checked runtime error if the array2b or its
 *        blocks have already been freed
 */
void UArray2b_free (T *array2b) 
{
        assert(array2b != NULL && *array2b != NULL);
        /* purely for style (readability) and the 80 characters */
        int blocksize = (*array2b)->blocksize;
        
        int block_width = ( (*array2b)->width + blocksize - 1) / blocksize;
        int block_height = ( (*array2b)->height + blocksize - 1) / blocksize;

        /* iterate through the 2D array and free each block */
        for (int i = 0; i < block_width; i++) {
                for (int j = 0; j < block_height; j++) {
                        UArray_T *block = UArray2_at((*array2b)->blocks, i, j);
                        UArray_free(block);
                }
        }
        
        /* free the actual 2d array and the container of all of the blocks */
        UArray2_free(&((*array2b)->blocks));
        FREE(*array2b);
}


/*
 * Name: UArray2b_width
 * 
 * Description: Returns the width of the UArray2b.
 *
 * Parameters:
 *           T array2b: the UArray2b struct
 *        
 * Returns: The width of the UArray2b
 * 
 * Expects: a valid array2b struct that has the width field filled in
 * 
 * Notes: checked runtime error for invalid array2b struct 
 */
int UArray2b_width (T array2b)
{
        assert(array2b != NULL);
        return array2b->width;
}


/*
 * Name: UArray2b_height
 * 
 * Description: Returns the height of the UArray2b.
 *
 * Parameters:
 *           T array2b: the UArray2b struct
 *        
 * Returns: The height of the UArray2b
 * 
 * Expects: a valid array2b struct that has the width field filled in
 * 
 * Notes: checked runtime error for invalid array2b struct 
 */
int UArray2b_height (T array2b)
{
        assert(array2b != NULL);
        return array2b->height;
}


/*
 * Name: UArray2b_size
 * 
 * Description: Returns the size of each element in the UArray2b.
 *
 * Parameters:
 *           T array2b: the UArray2b struct
 *        
 * Returns: The size of the elements of the UArray2b
 * 
 * Expects: a valid array2b struct that has the width field filled in
 * 
 * Notes: checked runtime error for invalid array2b struct 
 */
int UArray2b_size (T array2b)
{
        assert(array2b != NULL);
        return array2b->size;
}


/*
 * Name: UArray2b_blocksize
 * 
 * Description: Returns the blocksize of the UArray2b.
 *
 * Parameters:
 *           T array2b: the UArray2b struct
 *        
 * Returns: The blocksize of the blocks of the UArray2b
 * 
 * Expects: a valid array2b struct that has the width field filled in
 * 
 * Notes: checked runtime error for invalid array2b struct 
 */
int UArray2b_blocksize(T array2b)
{
        assert(array2b != NULL);
        return array2b->blocksize;
}


/*
 * Name: UArray2b_at
 * 
 * Description: Returns a pointer to the element at the specified column and row
 * in the UArray2b.
 *
 * Parameters:
 *           T array2b: the UArray2b structure
 *           int column: the column index
 *           int row: the row index
 *        
 * Returns: a pointer to the element at the specified column and row
 * 
 * Expects: a valid position inside the given uarray2b
 * 
 * Notes: results in a checked runtime error if invalid uarray2b or if the 
 * element position is invalid
 */
void *UArray2b_at(T array2b, int column, int row)
{
        assert(array2b != NULL);
        assert(column >= 0 && column < array2b->width && row >= 0 && 
               row < array2b->height);

        int blocksize = array2b->blocksize;

        int block_col = column / blocksize;
        int block_row = row / blocksize;

        /* formula to get the index of the element. First get the index of the 
        block and then the index of the cell inside the block */
        int index = (row % blocksize) * blocksize + (column % blocksize);

        /* access the block and the wanted element */
        UArray_T *block = UArray2_at(array2b->blocks, block_col, block_row);
        return UArray_at(*block, index);
}


/*
 * Name: UArray2b_map
 * 
 * Description: Applies the provided apply function to each element in the UArray2b,
 * iterating in row-major order. The apply function takes as arguments the column
 * index, row index, the UArray2b, a pointer to the element, and a closure pointer.
 *
 * Parameters:
 *           T array2b: the UArray2b structure
 *           void apply(int col, int row, T array2b, void *elem, void *cl): the apply
 *               function to be applied to each element
 *           void *cl: a closure pointer
 *        
 * Returns: None
 * 
 * Expects: array2b != NULL, apply != NULL
 * 
 * Notes: The apply function is responsible for any modifications to the elements
 * in the UArray2b.
 */
void UArray2b_map(T array2b, void apply(int col, int row, T array2b, void *elem,
                  void *cl), void *cl)
{
        assert(array2b != NULL);
        assert(apply != NULL);

        int blocksize = array2b->blocksize;
        /* array2b->width + blocksize - 1 allows us to visit non-full blocks */
        int block_width = (array2b->width + blocksize - 1) / blocksize;
        int block_height = (array2b->height + blocksize - 1) / blocksize;

        /* iterate throuhg the blocks and for each block iterate through all of
        the elements */
        for (int block_row = 0; block_row < block_height; block_row++) {
                for (int block_col = 0; block_col < block_width; block_col++) {
                        for (int i = 0; i < blocksize; i++) {
                                for (int j = 0; j < blocksize; j++) {
                                        int col = block_col * blocksize + j;
                                        int row = block_row * blocksize + i;
                                        /* check for right dimensions (unused
                                        cells)*/
                                        if (col < array2b->width && 
                                            row < array2b->height) {
                                                apply(col, row, array2b, 
                                                UArray2b_at(array2b, col, row),
                                                cl);
                                        }
                                }
                        }
                }
        }
}
