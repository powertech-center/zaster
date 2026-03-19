/*
 * Copyright (c) 2026-present, PowerTech.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ZASTER_MEMORY_H
#define ZASTER_MEMORY_H

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Memory: page-level OS allocation
 *
 * Low-level aligned allocation directly from the OS.
 * All sizes are expressed in pages (multiples of ZASTER_PAGE_SIZE).
 * ========================================================================= */

/** Size of one memory page in bytes (64 KB). */
#define ZASTER_PAGE_SIZE ((size_t)(64 * 1024))

/**
 * Returns the number of pages needed to hold `size` bytes.
 * Equivalent to ceil(size / ZASTER_PAGE_SIZE).
 */
ZASTER_INLINE size_t zaster_size_to_pages(size_t size) {
    return (size + ZASTER_PAGE_SIZE - 1) / ZASTER_PAGE_SIZE;
}

/**
 * Allocate `count` pages (count * ZASTER_PAGE_SIZE bytes) from the OS.
 *
 * Returns a pointer to the allocated memory, or NULL if count == 0 or
 * allocation fails.
 *
 * Must be freed with zaster_pages_free() passing the same count.
 */
ZASTER_API void* zaster_pages_alloc(size_t count);

/**
 * Free memory allocated by zaster_pages_alloc().
 *
 * Returns ZASTER_TRUE on success, ZASTER_FALSE on failure.
 */
ZASTER_API ZasterBool zaster_pages_free(void* ptr, size_t count);

/* =========================================================================
 * Memory: Segment
 *
 * A Segment footer is stored at the VERY END of each OS-allocated block:
 *
 *   [ usable data ... | ZasterSegment ]
 *    ^start            ^pointer returned by zaster_segment_new()
 *
 * Key properties:
 *   - Do NOT allocate ZasterSegment on the stack or copy it by value.
 *     It is always embedded at the end of a large OS-allocated block.
 *   - `start` points to the first usable byte of the block.
 *   - `prev` forms a singly-linked chain of segments (NULL = first segment).
 *   - The usable data region is: [start, seg)  (exclusive end).
 * ========================================================================= */

/** Size of the segment footer, rounded up to a 16-byte boundary. */
#define ZASTER_SEGMENT_FOOTER_SIZE ((sizeof(ZasterSegment) + 15u) & ~(size_t)15u)

/**
 * Segment footer. Lives at the end of each OS-allocated block.
 */
typedef struct ZasterSegment {
    /** Pointer to the first usable byte of this block. */
    uint8_t* start;
    /** Previous segment in the chain, or NULL if this is the first. */
    struct ZasterSegment* prev;
} ZasterSegment;

/* Compile-time layout checks for binary compatibility across languages. */
ZASTER_STATIC_ASSERT(sizeof(ZasterSegment) == 2 * sizeof(void*),
    "ZasterSegment must be exactly two pointers wide");
ZASTER_STATIC_ASSERT(offsetof(ZasterSegment, start) == 0,
    "ZasterSegment.start must be at offset 0");
ZASTER_STATIC_ASSERT(offsetof(ZasterSegment, prev) == sizeof(void*),
    "ZasterSegment.prev must be at offset sizeof(void*)");

/**
 * Allocate a new segment large enough to hold at least `data_size` bytes.
 * Links the new segment to `prev` (may be NULL).
 *
 * Returns a pointer to the segment footer at the end of the block,
 * or NULL on allocation failure.
 *
 * Must be freed with zaster_segment_free().
 */
ZASTER_API ZasterSegment* zaster_segment_new(size_t data_size, ZasterSegment* prev);

/**
 * Free a segment. If `recursive` is non-zero, frees the entire chain
 * by following `prev` links.
 *
 * If `seg` is NULL, returns 1 immediately (no-op).
 *
 * Returns ZASTER_TRUE if all OS deallocations succeeded.
 * Returns ZASTER_FALSE if any deallocation failed.
 *
 * After this call `seg` is invalid and must not be used.
 */
ZASTER_API ZasterBool zaster_segment_free(ZasterSegment* seg, ZasterBool recursive);

/* =========================================================================
 * Memory: Allocator
 *
 * ZasterAllocator is a bump allocator backed by a chain of segments.
 * It only allocates, never frees individual items.
 * All memory is released at once via zaster_allocator_free().
 *
 * Layout:
 *   - `cur`     — next free byte (bump pointer)
 *   - `segment` — footer of the current segment; doubles as end-of-data
 *                 marker (cur <= segment means there is space left)
 *
 * Do NOT copy ZasterAllocator by value. Always pass by pointer.
 * ========================================================================= */

/**
 * Bump allocator. Must be initialized with zaster_allocator_init()
 * and released with zaster_allocator_free().
 */
typedef struct ZasterAllocator {
    /** Next free byte in the current segment. */
    uint8_t* cur;
    /** Footer of the current segment (= end of usable data), or NULL. */
    ZasterSegment* segment;
} ZasterAllocator;

/* Compile-time layout checks for binary compatibility across languages. */
ZASTER_STATIC_ASSERT(sizeof(ZasterAllocator) == 2 * sizeof(void*),
    "ZasterAllocator must be exactly two pointers wide");
ZASTER_STATIC_ASSERT(offsetof(ZasterAllocator, cur) == 0,
    "ZasterAllocator.cur must be at offset 0");
ZASTER_STATIC_ASSERT(offsetof(ZasterAllocator, segment) == sizeof(void*),
    "ZasterAllocator.segment must be at offset sizeof(void*)");

/**
 * Initialize an allocator. No memory is allocated until the first
 * zaster_alloc() or zaster_reserve() call.
 */
ZASTER_INLINE void zaster_allocator_init(ZasterAllocator* allocator) {
    allocator->cur     = (uint8_t*)0;
    allocator->segment = (ZasterSegment*)0;
}

/**
 * Release all memory held by the allocator.
 *
 * Aborts the process if any OS deallocation fails.
 * After this call the allocator is in the same state as after init.
 */
ZASTER_INLINE void zaster_allocator_free(ZasterAllocator* allocator) {
    if (ZASTER_UNLIKELY(allocator->segment != (ZasterSegment*)0)) {
        if (!zaster_segment_free(allocator->segment, ZASTER_TRUE)) {
            ZASTER_FATAL("failed to free allocator segments");
        }
        allocator->cur     = (uint8_t*)0;
        allocator->segment = (ZasterSegment*)0;
    }
}

/**
 * Grow the allocator by adding a new segment of at least `size` bytes.
 * Cold path — separated so the hot path stays small and register-friendly.
 *
 * Aborts on allocation failure.
 */
ZASTER_API ZASTER_COLD ZASTER_NOINLINE
ZasterSegment* zaster_allocator_grow(size_t size, ZasterSegment* prev);

/**
 * Reserve `size` bytes and return a pointer to the start of the region.
 * Does NOT advance cur — caller decides whether to commit.
 *
 * Hot path: cur + size fits in the current segment — returns cur.
 * Cold path: allocates a new segment (aborts on OOM).
 */
ZASTER_INLINE uint8_t* zaster_reserve(ZasterAllocator* allocator, size_t size) {
    uint8_t* next = allocator->cur + size;
    if (ZASTER_LIKELY(next <= (uint8_t*)allocator->segment)) {
        return allocator->cur;
    }
    ZasterSegment* seg = zaster_allocator_grow(size, allocator->segment);
    allocator->cur     = seg->start;
    allocator->segment = seg;
    return allocator->cur;
}

/**
 * Allocate `size` bytes and return a pointer to the allocated region.
 * Advances cur by `size`.
 *
 * Hot path: cur + size fits in the current segment.
 * Cold path: allocates a new segment (aborts on OOM).
 */
ZASTER_INLINE uint8_t* zaster_alloc(ZasterAllocator* allocator, size_t size) {
    uint8_t* next = allocator->cur + size;
    if (ZASTER_LIKELY(next <= (uint8_t*)allocator->segment)) {
        uint8_t* ptr   = allocator->cur;
        allocator->cur = next;
        return ptr;
    }
    ZasterSegment* seg = zaster_allocator_grow(size, allocator->segment);
    uint8_t* ptr       = seg->start;
    allocator->cur     = ptr + size;
    allocator->segment = seg;
    return ptr;
}

#ifdef __cplusplus
}
#endif

#endif /* ZASTER_MEMORY_H */
