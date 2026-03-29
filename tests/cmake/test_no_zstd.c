#include "lib/defs.h"
#include "lib/memory.h"
#include "lib/tar.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    int ok = 1;

    /* zaster memory: page allocation */
    void *page = zaster_pages_alloc(1);
    if (!page) { fprintf(stderr, "FAIL: zaster_pages_alloc(1) returned NULL\n"); ok = 0; }
    else {
        printf("OK: zaster_pages_alloc(1) = %p\n", page);
        ZasterBool freed = zaster_pages_free(page, 1);
        if (freed != ZASTER_TRUE) { fprintf(stderr, "FAIL: zaster_pages_free\n"); ok = 0; }
        else                       { printf("OK: zaster_pages_free succeeded\n"); }
    }

    /* tar header size */
    if (sizeof(ZasterTarHeader) != ZASTER_TAR_BLOCK_SIZE) {
        fprintf(stderr, "FAIL: sizeof(ZasterTarHeader) = %zu\n", sizeof(ZasterTarHeader));
        ok = 0;
    } else {
        printf("OK: sizeof(ZasterTarHeader) = %d\n", ZASTER_TAR_BLOCK_SIZE);
    }

    /* allocator */
    ZasterAllocator alloc;
    zaster_allocator_init(&alloc);
    uint8_t *ptr = zaster_alloc(&alloc, 64);
    if (!ptr) { fprintf(stderr, "FAIL: zaster_alloc returned NULL\n"); ok = 0; }
    else {
        memset(ptr, 0xAA, 64);
        printf("OK: zaster_alloc(64) = %p\n", (void*)ptr);
    }
    zaster_allocator_free(&alloc);
    printf("OK: allocator freed\n");

    printf("\n%s\n", ok ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return ok ? 0 : 1;
}
