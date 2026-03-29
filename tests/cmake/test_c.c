#include "zaster.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    int ok = 1;

    /* zstd version */
    unsigned ver = ZSTD_versionNumber();
    if (ver == 0) { fprintf(stderr, "FAIL: ZSTD_versionNumber() == 0\n"); ok = 0; }
    else          { printf("OK: ZSTD_versionNumber() = %u\n", ver); }

    const char *vs = ZSTD_versionString();
    if (!vs || vs[0] == '\0') { fprintf(stderr, "FAIL: ZSTD_versionString() empty\n"); ok = 0; }
    else                       { printf("OK: ZSTD_versionString() = %s\n", vs); }

    /* zstd compress/decompress round-trip */
    const char *input = "Hello, Zaster! This is a test of CMake integration.";
    size_t input_len = strlen(input);
    size_t bound = ZSTD_compressBound(input_len);

    char compressed[4096];
    char decompressed[4096];

    size_t csize = ZSTD_compress(compressed, sizeof(compressed), input, input_len, 1);
    if (ZSTD_isError(csize)) {
        fprintf(stderr, "FAIL: ZSTD_compress: %s\n", ZSTD_getErrorName(csize));
        ok = 0;
    } else {
        printf("OK: ZSTD_compress: %zu -> %zu bytes\n", input_len, csize);

        size_t dsize = ZSTD_decompress(decompressed, sizeof(decompressed), compressed, csize);
        if (ZSTD_isError(dsize)) {
            fprintf(stderr, "FAIL: ZSTD_decompress: %s\n", ZSTD_getErrorName(dsize));
            ok = 0;
        } else if (dsize != input_len || memcmp(decompressed, input, input_len) != 0) {
            fprintf(stderr, "FAIL: round-trip mismatch\n");
            ok = 0;
        } else {
            printf("OK: round-trip verified (%zu bytes)\n", dsize);
        }
    }

    /* zaster memory: page allocation */
    void *page = zaster_pages_alloc(1);
    if (!page) { fprintf(stderr, "FAIL: zaster_pages_alloc(1) returned NULL\n"); ok = 0; }
    else {
        printf("OK: zaster_pages_alloc(1) = %p\n", page);
        ZasterBool freed = zaster_pages_free(page, 1);
        if (freed != ZASTER_TRUE) { fprintf(stderr, "FAIL: zaster_pages_free\n"); ok = 0; }
        else                       { printf("OK: zaster_pages_free succeeded\n"); }
    }

    /* zaster tar: struct size */
    if (sizeof(ZasterTarHeader) != ZASTER_TAR_BLOCK_SIZE) {
        fprintf(stderr, "FAIL: sizeof(ZasterTarHeader) = %zu, expected %d\n",
                sizeof(ZasterTarHeader), ZASTER_TAR_BLOCK_SIZE);
        ok = 0;
    } else {
        printf("OK: sizeof(ZasterTarHeader) = %d\n", ZASTER_TAR_BLOCK_SIZE);
    }

    /* zaster memory: allocator */
    ZasterAllocator alloc;
    zaster_allocator_init(&alloc);
    uint8_t *ptr = zaster_alloc(&alloc, 128);
    if (!ptr) { fprintf(stderr, "FAIL: zaster_alloc returned NULL\n"); ok = 0; }
    else {
        memset(ptr, 0xAB, 128);
        printf("OK: zaster_alloc(128) = %p\n", (void*)ptr);
    }
    zaster_allocator_free(&alloc);
    printf("OK: zaster_allocator_free succeeded\n");

    printf("\n%s\n", ok ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return ok ? 0 : 1;
}
