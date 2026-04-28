/*
 * mm.c
 * 
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "",
    /* First member's full name */
    "",
    /* First member's email address */
    "",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/* 
 * mm_malloc 
 */
void *mm_malloc(size_t size)
{
    return NULL;
}

/*
 * mm_free 
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc 
 */
void *mm_realloc(void *ptr, size_t size)
{
    return NULL;
}

/*
 * output a block - for use in heapcheck 
 */
static void print_block(int request_id, int payload)
{
  printf("\n$BLOCK %d %d\n", request_id, payload); 
}

/*
 * mm_heapcheck 
 */
void mm_heapcheck()
{
} 


















