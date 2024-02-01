/*
 * mm.c
 *
 * Name: UC Choudhary
 *
 * First Commit Comment just added my name.
 * 
 * 
 *
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "mm.h"
#include "memlib.h"

// #define DEBUG

#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#else
#define dbg_printf(...)
#define dbg_assert(...)
#endif

#ifdef DRIVER
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mm_memset
#define memcpy mm_memcpy
#endif

#define ALIGNMENT 16
#define HEADER_SIZE 4
#define FOOTER_SIZE 8
#define INITIAL_HEAP_SIZE 8

//rounds up to the nearest multiple of ALIGNMENT
static size_t align(size_t x)
{
    return ALIGNMENT * ((x+ALIGNMENT-1)/ALIGNMENT);
}

static inline size_t PACK(size_t size, int alloc) {
    return size | alloc;
}

static inline void PUT(void* p, size_t val) {
    *((size_t*)p) = val;
}

static inline size_t GET(const void* p) {
    return *((size_t*)p);
}

static inline void* HDRP(void* bp) {
    return (char*)bp - HEADER_SIZE;
}

static inline void* FTRP(void* bp) {
    return (char*)bp + GET_SIZE(HDRP(bp)) - HEADER_SIZE - FOOTER_SIZE;
}

static inline size_t GET_SIZE(const void* p) {
    return GET(p) & ~0x7;
}

static inline int GET_ALLOC(const void* p) {
    return GET(p) & 0x1;
}

static inline void* NEXT_BLKP(void* bp) {
    return (char*)bp + GET_SIZE((char*)bp - HEADER_SIZE);
}

static inline void* PREV_BLKP(void* bp) {
    return (char*)bp - GET_SIZE((char*)bp - HEADER_SIZE - FOOTER_SIZE);
}

static inline int MAX(int x, int y) {
    return x > y ? x : y;
}

static char *heap_listp; // Pointer to first block

/*
 * mm_init: returns false on error, true on success.
 */

bool mm_init(void) 
{
    if ((heap_listp = mem_sbrk(4*HEADER_SIZE)) == (void*)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*HEADER_SIZE), PACK(FOOTER_SIZE, 1));
    PUT(heap_listp + (2*HEADER_SIZE), PACK(FOOTER_SIZE, 1));
    PUT(heap_listp + (3*HEADER_SIZE), PACK(0,1));
    heap_listp += (2*HEADER_SIZE);
    if (extend_heap(INITIAL_HEAP_SIZE/HEADER_SIZE) == NULL)
        return false;
    return true;
}

// extend_heap - Extend heap with free block and return its block pointer
static void *extend_heap(uint32_t words) 
{
    char *bp;
    size_t size;
    size = (words % 2) ? (words+1) * HEADER_SIZE : words * HEADER_SIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
    return coalesce(bp);
}

// coalesce - Boundary tag coalescing. Return ptr to coalesced block
static void *coalesce(void *bp) 
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE (HDRP(bp));
    
    if (prev_alloc && next_alloc) {
        return bp;
    }
    
    else if (prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size,0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
  return bp;
}

/*
 * malloc
 */
void* malloc(size_t size) {
    mm_checkheap(__LINE__);
    uint64_t total_block_size = (uint64_t)align(size + HEADER_SIZE + FOOTER_SIZE);
    uint64_t* current_block_ptr;
    for (current_block_ptr = prologue; get_total_block_size(current_block_ptr) > 0; current_block_ptr = get_next_block(current_block_ptr)) {
        if (is_block_allocated(current_block_ptr) == 0 && get_total_block_size(current_block_ptr) >= total_block_size) {
            split_and_allocate_block(current_block_ptr);
            mm_checkheap(__LINE__);
            return get_payload_ptr(current_block_ptr);
        }
    }
    current_block_ptr = expand_heap(total_block_size);
    mm_checkheap(__LINE__);
    return get_payload_ptr(current_block_ptr);
}

/*
 * free
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);   
}


/*
 * realloc
 */
void *mm_realloc(void *ptr, uint32_t size)
{
    void *newp;
    uint32_t copySize;
    newp = mm_malloc(size);
    if (newp == NULL) {
        printf("ERROR: mm_malloc failed in mm_realloc\n");
        exit(1);
    }
    copySize = GET_SIZE(HDRP(ptr));
    if (size < copySize) {
        copySize = size;
    }
    memcpy(newp, ptr, copySize);
    mm_free(ptr);
    return newp;
}
/*
 * calloc
 * This function is not tested by mdriver, and has been implemented for you.
 */
void* calloc(size_t nmemb, size_t size)
{
    void* ptr;
    size *= nmemb;
    ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

/*
 * Returns whether the pointer is in the heap.
 * May be useful for debugging.
 */
static bool in_heap(const void* p)
{
    return p <= mm_heap_hi() && p >= mm_heap_lo();
}

/*
 * Returns whether the pointer is aligned.
 * May be useful for debugging.
 */
static bool aligned(const void* p)
{
    // The address is aligned if it is a multiple of ALIGNMENT
    // Call the align function to align the value of 'ip'
    // Check if the aligned value is equal to the original value of 'ip'
    size_t ip = (size_t) p;
    return align(ip) == ip;
}

/*
 * mm_checkheap
 * You call the function via mm_checkheap(__LINE__)
 * The line number can be used to print the line number of the calling
 * function where there was an invalid heap.
 */
void mm_checkheap(int verbose) 
{
    void *bp = heap_listp;
    if (verbose) {
        printf("Heap (%p):\n", heap_listp);
    }
    if ((GET_SIZE(HDRP(heap_listp)) != FOOTER_SIZE) || !GET_ALLOC(HDRP(heap_listp))) {
        printf("Bad prologue header\n");
    }
    checkblock(heap_listp);
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose)  {
            printblock(bp);
        }
        checkblock(bp);
    }    
    if (verbose) {
        printblock(bp);
    }
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
        printf("Bad epilogue header\n");
    }
}
// following are the functions that I have added
// according to the malloc hint announcement.
// to implement "clean code" in a modular way.
uint64_t* prologue;

uint64_t get_total_block_size(uint64_t* block_ptr) {
    return *block_ptr;
}

uint64_t is_block_allocated(uint64_t* block_ptr) {
    return *block_ptr & 1;
}

uint64_t* get_next_block(uint64_t* block_ptr) {
    return (uint64_t*)((char*)block_ptr + get_total_block_size(block_ptr));
}

void* get_payload_ptr(uint64_t* block_ptr) {
    return (void*)(block_ptr + 1);
}

void split_and_allocate_block(uint64_t* block_ptr) {
    *block_ptr |= 1; // Mark the block as allocated
}


/*
 * The expand_heap function is designed to increase the heap size dynamically 
 * when the memory allocator cannot find a suitable block of memory to satisfy 
 * an allocation request. Unlike the extend_heap function, which was called 
 * during the initialization of the memory management system to set up an initial 
 * heap size or in specific scenarios where a significant heap extension is needed, 
 * expand_heap is directly tied to the allocator's runtime operations.
*/

// Prototype for mem_sbrk, assuming it's similar to sbrk but specific to your memory management system.
extern void *mem_sbrk(intptr_t incr);

// Function to expand the heap by the requested size.
// Returns a pointer to the newly allocated block or NULL if the allocation fails.
uint64_t* expand_heap(size_t size) {
    // Align the requested size
    size = align(size);

    // Request more memory from the OS
    void* new_block = mem_sbrk(size);
    if (new_block == (void*)-1) {
        return NULL; // Failed to allocate more memory
    }

    // Assuming the last block is an epilogue block, we convert it into a header for the new block.
    // Adjust the header of the new block. The size is already aligned, and we mark it as free (0).
    uint64_t* header = (uint64_t*)((char*)new_block - HEADER_SIZE);
    *header = size;

    // Set up the footer for the new block
    uint64_t* footer = (uint64_t*)((char*)header + size - FOOTER_SIZE);
    *footer = size;

    // Create a new epilogue header after the new block
    uint64_t* new_epilogue = (uint64_t*)((char*)footer + FOOTER_SIZE);
    *new_epilogue = PACK(0, 1); // Size 0, marked as allocated

    // Return a pointer to the payload of the new block
    return (uint64_t*)((char*)header + HEADER_SIZE);
}

