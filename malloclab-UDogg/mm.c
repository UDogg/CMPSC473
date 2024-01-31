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
#define HEADER_SIZE 8
#define FOOTER_SIZE 8
#define INITIAL_HEAP_SIZE (1<<12)// 4KB


// rounds up to the nearest multiple of ALIGNMENT
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


/*
 * mm_init: returns false on error, true on success.
 */
bool mm_init(void)
{
    // IMPLEMENT THIS
    // Request space from mem_sbrk for the initial empty heap
    char* heap_listp = (char*)mem_sbrk(4*ALIGNMENT); // Adjust the size as needed
    if (heap_listp == (void*)-1) {
        return false; // mem_sbrk failed, return error
    }
    
    // Initialize the heap area, including prologue and epilogue blocks
    // or any other necessary initialization for the block format.
    // Example for a simple prologue/epilogue initialization:
    *(size_t *)(heap_listp) = 0; // Alignment padding
    *(size_t *)(heap_listp + (1*ALIGNMENT)) = PACK(ALIGNMENT, 1); // Prologue header
    *(size_t *)(heap_listp + (2*ALIGNMENT)) = PACK(ALIGNMENT, 1); // Prologue footer
    *(size_t *)(heap_listp + (3*ALIGNMENT)) = PACK(0, 1); // Epilogue header
    heap_listp += (2*ALIGNMENT);

    // Extend the empty heap with a free block of INITIAL_HEAP_SIZE bytes
    // to avoid fragmentation issues later.
    if (extend_heap(INITIAL_HEAP_SIZE/ALIGNMENT) == NULL) {
        return false; // Unable to extend heap, return error
    }
    return true;
}

//coalesce function
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            // Case 1
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      // Case 2
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) {      // Case 3
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else {                                     // Case 4
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    return bp;
}



/*
* *extend_heap(size_t words)
* returns a pointer to the newly created free block
* helper function for mm_init
*/

// extend_heap - Extend heap with free block and return its block pointer
// coalesce - Boundary tag coalescing. Return ptr to coalesced block
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    // Align the requested size to the word boundary
    size = (words % 2) ? (words + 1) * ALIGNMENT : words * ALIGNMENT;

    // Request more memory from the OS
    bp = mem_sbrk(size);
    if ((long)bp == -1) {
        return NULL; // Failed to extend the heap
    }

    // Initialize the new free block's header and the new epilogue header
    PUT(HDRP(bp), PACK(size, 0)); // Free block header
    PUT(FTRP(bp), PACK(size, 0)); // Free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // New epilogue block header

    // Return the pointer to the new free block
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
void free(void* ptr) {
    if (ptr == NULL) return; // Standard behavior of free

    // Convert payload pointer to block pointer (assuming header is right before payload)
    void* block_ptr = get_block_header(ptr);

    // Mark the block as free
    size_t size = GET_SIZE(HDRP(block_ptr));
    PUT(HDRP(block_ptr), PACK(size, 0)); // Update header to mark as free
    PUT(FTRP(block_ptr), PACK(size, 0)); // Update footer to mark as free

    // Coalesce with adjacent free blocks
    block_ptr = coalesce(block_ptr);

    // Add to the free list
    insert_free_block(block_ptr);
}


/*
 * realloc
 */
void* realloc(void* oldptr, size_t size)
{
    // IMPLEMENT THIS
    return NULL;
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
bool mm_checkheap(int line_number)
{
#ifdef DEBUG
    // Write code to check heap invariants here
    // IMPLEMENT THIS
#endif // DEBUG
    return true;
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

