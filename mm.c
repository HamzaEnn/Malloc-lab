/*
 * mm.c
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define CHUNKSIZE (1 << 12) /* initial heap size (bytes) */
#define WSIZE 4             /* word size (bytes) */
#define DSIZE 8             /* doubleword size (bytes) */
#define OVERHEAD 8          /* overhead of header and footer (bytes) */

struct malloc_stc {
  struct malloc_stc *next;
  struct malloc_stc *prev;
  int size;
  char alloc;
  char *buffer;
};

#define HEADER_SIZE (sizeof(struct malloc_stc))

/* Global variables */
static char *pHeapList; /* pointer to first block */

void *extend_heap_size(size_t size, void *pPrev) {

  void *extended = mem_sbrk(size + HEADER_SIZE);
  if (extended == (void *)-1)
    return NULL;

  struct malloc_stc *head = (struct malloc_stc *)extended;
  head->next = NULL;
  head->prev = pPrev;
  head->size = size;
  head->alloc = 0;
  head->buffer = extended + HEADER_SIZE;

  return head;
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
  printf("mm_init\n");

  pHeapList = extend_heap_size(CHUNKSIZE, NULL);
  if (pHeapList == NULL) {
    return -1;
  }

  printf("  Heap initialized with first block at %p\n", pHeapList);
  return 0;
}

void *find_fit(size_t size) {
  struct malloc_stc *current = (struct malloc_stc *)pHeapList;
  struct malloc_stc *fit = NULL;
  while (1) {
    if (!current->alloc && current->size >= size) {
      fit = current;
      break;
    }
    if (current->next == NULL) {
      break;
    }
    current = current->next;
  }

  if (fit == NULL) {
    if (!current->alloc) {
      if (mem_sbrk(size - current->size) == (void *)-1) {
        return NULL;
      }

      fit = current;
      fit->size = size;
    } else {
      fit = extend_heap_size(size, current);
      if (fit == NULL) {
        return NULL;
      }

      current->next = fit;
    }
  }

  return fit;
}

void split(struct malloc_stc *pSpace, int allocSize) {
  // If after the split we have enough room for a free space
  if (pSpace->size > allocSize + HEADER_SIZE) {
    struct malloc_stc *pFreeSpace =
        (struct malloc_stc *)(pSpace->buffer + allocSize);

    pFreeSpace->alloc = 0;
    pFreeSpace->buffer = (void *)pFreeSpace + HEADER_SIZE;
    pFreeSpace->next = pSpace->next;
    pFreeSpace->prev = pSpace;
    pFreeSpace->size = pSpace->size - allocSize - HEADER_SIZE;

    pSpace->alloc = 1;
    pSpace->next = pFreeSpace;
    pSpace->size = allocSize;
  }
  pSpace->alloc = 1;
}

/*
 * mm_malloc
 */
void *mm_malloc(size_t size) {
  printf("mm_malloc: Requesting %zu bytes\n", size);
  size_t adapted_size = ALIGN(size);

  // Find free space that fits
  struct malloc_stc *pFitSpace = find_fit(adapted_size);
  if (pFitSpace == NULL) {
    return NULL;
  }

  // Split
  split(pFitSpace, adapted_size);

  printf("  mm_malloc: Allocating %zu bytes at %p\n", adapted_size,
         pFitSpace->buffer);
  mm_heapcheck();
  return pFitSpace->buffer;
}

struct malloc_stc *merge(struct malloc_stc *pPrev, struct malloc_stc *pNext) {

  pPrev->next = pNext->next;
  printf("  Merging blocks %p and %p\n", pPrev, pNext);

  if (pPrev->next != NULL) {
    (pPrev->next)->prev = pPrev;
  }

  // Size
  pPrev->size += pNext->size + HEADER_SIZE;

  return pPrev;
}

int findPtr(void *ptr) {
  struct malloc_stc *pCurrent = (struct malloc_stc *)(pHeapList);
  int idx = 1;

  while (pCurrent != NULL) {
    printf("  Checking block %d: %p\n", idx, pCurrent);
    if (pCurrent == ptr) {
      return idx;
    }
    pCurrent = pCurrent->next;
    ++idx;
  }

  return 0;
}

/*
 * mm_free
 */
void mm_free(void *ptr) {
  struct malloc_stc *pSpace = (struct malloc_stc *)(ptr - HEADER_SIZE);
  printf("mm_free: Releasing %p. Idx %u\n", pSpace, findPtr(pSpace));
  pSpace->alloc = 0;

  // Coalesce
  struct malloc_stc *pPrev = pSpace->prev;
  printf("  Previous block: %p\n", pPrev);

  if (pPrev != NULL && !pPrev->alloc) {
    printf("  Merging with previous block\n");
    pSpace = merge(pPrev, pSpace);
  }

  struct malloc_stc *pNext = pSpace->next;
  printf("  Next block: %p\n", pNext);
  if (pNext != NULL && !pNext->alloc) {
    printf("  Merging with next block\n");
    merge(pSpace, pNext);
  }
}

void mergeMultipleSpaces(struct malloc_stc *pLeftist,
                         struct malloc_stc *pRightest, int totalSize,
                         int targettedSize) {
  // Merging
  pLeftist->next = pRightest->next;
  (pLeftist->next)->prev = pLeftist;

  // Size
  pLeftist->size += totalSize + HEADER_SIZE;
}

struct malloc_stc *checkNeighbourhood(struct malloc_stc *pOriginal,
                                      int targettedSize) {
  // check next free spaces
  int accSize = pOriginal->size;
  struct malloc_stc *pCurrent = pOriginal->next;
  struct malloc_stc *pLastNext = pOriginal;
  while (pCurrent == NULL && !pCurrent->alloc) {
    accSize += pCurrent->size + HEADER_SIZE;
    if (accSize >= targettedSize) {
      break;
    }

    pLastNext = pCurrent;
    pCurrent = pCurrent->next;
  }

  // check previous spaces
  pCurrent = pOriginal->prev;
  struct malloc_stc *pLastPrev = pOriginal;
  while (pCurrent == NULL && !pCurrent->alloc) {
    accSize += pCurrent->size + HEADER_SIZE;
    if (accSize >= targettedSize) {
      break;
    }

    pLastPrev = pCurrent;
    pCurrent = pCurrent->prev;
  }

  if (accSize >= targettedSize) {
  }
}

/*
 * mm_realloc
 */
void *mm_realloc(void *ptr, size_t size) {
  printf("mm_realloc: Requesting %zu bytes\n", size);

  struct malloc_stc *pOriginal = ptr - HEADER_SIZE;
  if (pOriginal->size < size) {
    // else, malloc, copy and free
    struct malloc_stc *pNew = mm_malloc(size) - HEADER_SIZE;
    memcpy(pNew->buffer, pOriginal->buffer, pOriginal->size);
    return pNew->buffer;
  } else if (pOriginal->size > size) {
    // split
    split(pOriginal, size);
  }
  return pOriginal->buffer;
}

/*
 * output a block - for use in heapcheck
 */
static void print_block(int request_id, int payload) {
  printf("\n$BLOCK %d %d\n", request_id, payload);
}

/*
 * mm_heapcheck
 */
void mm_heapcheck() {
  struct malloc_stc *pCurrent = (struct malloc_stc *)(pHeapList);

  while (pCurrent != NULL) {
    printf("  block: %p\n", pCurrent);
    printf("    size: %u\n", pCurrent->size);
    printf("    next: %p\n", pCurrent->next);
    printf("    prev: %p\n", pCurrent->prev);
    printf("    buffer: %p\n", pCurrent->buffer);
    printf("    alloc: %d\n", pCurrent->alloc);

    pCurrent = pCurrent->next;
  }
}
