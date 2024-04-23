/*
 *     a2plain.c
 *     Javier Gonzalez (jgonza20) and Cheng Li (cli01)
 *     2/23/24
 *     Locality
 *
 *     This file implements the A2Methods interface using plain arrays.
 *     It provides functions for creating, accessing, and manipulating 2D 
 *     arrays represented as plain arrays.
 *    
 */


#include <string.h>
#include <a2plain.h>
#include "uarray2.h"


typedef A2Methods_UArray2 A2;


/* Allocate and return a new A2Methods_UArray2 with the given width, height, and size */
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/* Allocate and return a new A2Methods_UArray2 with the given width, height, size, and blocksize */
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void)blocksize;
        return UArray2_new(width, height, size);
}


/* Free the memory allocated for the A2Methods_UArray2 pointed to by array2p */
/* Free the memory allocated for the A2Methods_UArray2 pointed to by array2p */
static void a2free(A2 * array2p)
{
        UArray2_free((UArray2_T *) array2p);
}


/* Return the width of the A2Methods_UArray2 array2 */
/* Return the width of the A2Methods_UArray2 array2 */
static int width(A2 array2)
{
        return UArray2_width(array2);
}


/* Return the height of the A2Methods_UArray2 array2 */
/* Return the height of the A2Methods_UArray2 array2 */
static int height(A2 array2)
{
        return UArray2_height(array2);
}


/* Return the size of each element in the A2Methods_UArray2 array2 */
/* Return the size of each element in the A2Methods_UArray2 array2 */
static int size(A2 array2)
{
        return UArray2_size(array2);
}


/* Always return 1 */
/* Always return 1 */
static int blocksize(A2 array2)
{
        (void)array2;
        return 1;
}


/* Return a pointer to the element at position (i, j) in the A2Methods_UArray2 
array2 */
/* Return a pointer to the element at position (i, j) in the A2Methods_UArray2 
array2 */
static A2Methods_Object *at(A2 array2, int i, int j)
{
        return UArray2_at(array2, i, j);
}


/* Apply the function apply to each element of the A2Methods_UArray2 uarray2 in
row-major order, passing cl as an additional argument */
/* Apply the function apply to each element of the A2Methods_UArray2 uarray2 in
row-major order, passing cl as an additional argument */
static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}


/* Apply the function apply to each element of the A2Methods_UArray2 uarray2 in
column-major order, passing cl as an additional argument */
/* Apply the function apply to each element of the A2Methods_UArray2 uarray2 in
column-major order, passing cl as an additional argument */
static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/******************************************************************************/
                        /* THIS PART WAS PROVIDED */
/******************************************************************************/
/*****************************************************************************/
                        /* THIS PART WAS PROVIDED */
/*****************************************************************************/
struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};


static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}


static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}


static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

/******************************************************************************/
                       /* BACK TO OUR IMPLEMENTATION */
/******************************************************************************/

/* Define the A2Methods_T struct for plain arrays */
static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,
        map_col_major, 
        NULL,
        map_row_major, /* map default */
        small_map_row_major,
        small_map_col_major,
        NULL,
        small_map_row_major, /* small map default */
};

/* exported pointer to the struct */

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
