/*
 * Copyright (c) 2026-present, PowerTech.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "memory.h"

/* =========================================================================
 * Platform: OS-level page allocation
 * ========================================================================= */

#if defined(_WIN32)

/* --- Windows: VirtualAlloc / VirtualFree -------------------------------- */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void* zaster_pages_alloc(size_t count) {
    if (count == 0) return NULL;
    void* p = VirtualAlloc(NULL, count * ZASTER_PAGE_SIZE,
                           MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return p;  /* NULL on failure */
}

ZasterBool zaster_pages_free(void* ptr, size_t count) {
    (void)count;
    if (ptr == NULL) return ZASTER_FALSE;
    return VirtualFree(ptr, 0, MEM_RELEASE) ? ZASTER_TRUE : ZASTER_FALSE;
}

#else

/* --- Unix / Linux / macOS: mmap / munmap -------------------------------- */

#include <sys/mman.h>

void* zaster_pages_alloc(size_t count) {
    if (count == 0) return NULL;
    void* p = mmap(NULL, count * ZASTER_PAGE_SIZE,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1, 0);
    return (p == MAP_FAILED) ? NULL : p;
}

ZasterBool zaster_pages_free(void* ptr, size_t count) {
    if (ptr == NULL) return ZASTER_FALSE;
    return (munmap(ptr, count * ZASTER_PAGE_SIZE) == 0) ? ZASTER_TRUE : ZASTER_FALSE;
}

#endif

/* =========================================================================
 * Segment
 * ========================================================================= */

ZasterSegment* zaster_segment_new(size_t data_size, ZasterSegment* prev) {
    size_t count = zaster_size_to_pages(data_size + ZASTER_SEGMENT_FOOTER_SIZE);
    uint8_t* block = (uint8_t*)zaster_pages_alloc(count);
    if (block == NULL) return NULL;

    ZasterSegment* seg = (ZasterSegment*) (block + count * ZASTER_PAGE_SIZE - ZASTER_SEGMENT_FOOTER_SIZE);
    seg->start = block;
    seg->prev  = prev;
    return seg;
}

ZasterBool zaster_segment_free(ZasterSegment* seg, ZasterBool recursive) {
    if (seg == NULL) return ZASTER_TRUE;

    ZasterBool ok = ZASTER_TRUE;
    ZasterSegment* current = seg;
    ZasterSegment* prev = recursive ? current->prev : NULL;
    uint8_t* start = current->start;

    for (;;) {
        size_t size = (uintptr_t)current + ZASTER_SEGMENT_FOOTER_SIZE - (uintptr_t)start;
        if (!zaster_pages_free(start, size / ZASTER_PAGE_SIZE)) ok = ZASTER_FALSE;
        if (prev == NULL) break;
        current = prev;
        prev = current->prev;
        start = current->start;
    }
    return ok;
}

/* =========================================================================
 * Allocator: cold path (grow)
 * ========================================================================= */

ZasterSegment* zaster_allocator_grow(size_t size, ZasterSegment* prev) {
    ZasterSegment* seg = zaster_segment_new(size, prev);
    if (seg == NULL) {
        ZASTER_FATAL("out of memory (requested %zu bytes)", size);
    }
    return seg;
}
