#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mem.h"
#include "uarray2b.h"
#include "uarray2.h"
#include "uarray.h"

void test_new();
void test_free();
void print_element(int col, int row, UArray2b_T array2b, void *elem, void *cl);
void populate(UArray2b_T array2b);
void populate_element(int col, int row, UArray2b_T array2b, void *elem, void *cl);

int main()
{
        UArray2b_T array2b = UArray2b_new(5, 5, sizeof(int), 2);
        UArray2b_map(array2b, print_element, NULL);
        test_new();
        printf("Test passed!\n");
        return 0;
}

void test_new() {
        UArray2b_T array2b = UArray2b_new(10, 10, sizeof(int), -1);
        assert(array2b != NULL);
        assert(UArray2b_width(array2b) == 10);
        assert(UArray2b_height(array2b) == 10);
        assert(UArray2b_size(array2b) == sizeof(int));
        assert(UArray2b_blocksize(array2b) == 2);
}

void test_free() {
        UArray2b_T array2b = UArray2b_new(10, 10, sizeof(int), 2);
        UArray2b_free(&array2b);
}


void print_element(int col, int row, UArray2b_T array2b, void *elem, void *cl) {

        (void)array2b;
        int *count = (int *)cl;
        printf("array[%d][%d] = %d\n", col, row, *(int *)elem);
        (*count)++;
}


void populate(UArray2b_T array2b)
{
        int count = 1;
        for (int row = 0; row < UArray2b_height(array2b); row++) {
        for (int col = 0; col < UArray2b_width(array2b); col++) {
            int *elem = (int *)UArray2b_at(array2b, col, row);
            *elem = count;
            count++;
        }
    }
}


void populate_element(int col, int row, UArray2b_T array2b, void *elem, void *cl) {
    (void)array2b;
    (void)col;
    (void)row;
    int *count = (int *)cl;
    *(int *)elem = *count;
    (*count)++;
}
