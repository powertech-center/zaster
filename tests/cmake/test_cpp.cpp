#include "zaster.hpp"
#include <cstdio>
#include <cstring>

int main() {
    int ok = 1;

    /* zstd namespace */
    unsigned ver = zstd::versionNumber();
    if (ver == 0) { fprintf(stderr, "FAIL: zstd::versionNumber() == 0\n"); ok = 0; }
    else          { printf("OK: zstd::versionNumber() = %u\n", ver); }

    const char *vs = zstd::versionString();
    if (!vs || vs[0] == '\0') { fprintf(stderr, "FAIL: zstd::versionString() empty\n"); ok = 0; }
    else                       { printf("OK: zstd::versionString() = %s\n", vs); }

    /* zstd round-trip via namespace */
    const char *input = "Hello from C++ Zaster test!";
    size_t input_len = std::strlen(input);
    size_t bound = zstd::compressBound(input_len);

    char compressed[4096];
    char decompressed[4096];

    size_t csize = zstd::compress(compressed, sizeof(compressed), input, input_len, 3);
    if (zstd::isError(csize)) {
        fprintf(stderr, "FAIL: zstd::compress: %s\n", zstd::getErrorName(csize));
        ok = 0;
    } else {
        printf("OK: zstd::compress: %zu -> %zu bytes\n", input_len, csize);

        size_t dsize = zstd::decompress(decompressed, sizeof(decompressed), compressed, csize);
        if (zstd::isError(dsize)) {
            fprintf(stderr, "FAIL: zstd::decompress: %s\n", zstd::getErrorName(dsize));
            ok = 0;
        } else if (dsize != input_len || std::memcmp(decompressed, input, input_len) != 0) {
            fprintf(stderr, "FAIL: round-trip mismatch\n");
            ok = 0;
        } else {
            printf("OK: round-trip verified (%zu bytes)\n", dsize);
        }
    }

    /* zstd context API */
    zstd::CCtx *cctx = zstd::createCCtx();
    if (!cctx) { fprintf(stderr, "FAIL: zstd::createCCtx() returned NULL\n"); ok = 0; }
    else {
        printf("OK: zstd::createCCtx() succeeded\n");
        zstd::freeCCtx(cctx);
    }

    /* zaster::memory::Allocator RAII */
    {
        zaster::memory::Allocator alloc;
        uint8_t *ptr = alloc.alloc(256);
        if (!ptr) { fprintf(stderr, "FAIL: Allocator::alloc returned NULL\n"); ok = 0; }
        else {
            std::memset(ptr, 0xCD, 256);
            printf("OK: zaster::memory::Allocator::alloc(256) = %p\n", static_cast<void*>(ptr));
        }

        uint8_t *ptr2 = alloc.reserve(64);
        if (!ptr2) { fprintf(stderr, "FAIL: Allocator::reserve returned NULL\n"); ok = 0; }
        else       { printf("OK: zaster::memory::Allocator::reserve(64) = %p\n", static_cast<void*>(ptr2)); }
    }
    printf("OK: Allocator RAII destructor completed\n");

    /* zaster::tar constants */
    if (zaster::tar::BLOCK_SIZE != 512) {
        fprintf(stderr, "FAIL: zaster::tar::BLOCK_SIZE = %d\n", zaster::tar::BLOCK_SIZE);
        ok = 0;
    } else {
        printf("OK: zaster::tar::BLOCK_SIZE = %d\n", zaster::tar::BLOCK_SIZE);
    }

    if (zaster::tar::TYPE_FILE != '0') {
        fprintf(stderr, "FAIL: zaster::tar::TYPE_FILE != '0'\n");
        ok = 0;
    } else {
        printf("OK: zaster::tar::TYPE_FILE = '%c'\n", zaster::tar::TYPE_FILE);
    }

    printf("\n%s\n", ok ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return ok ? 0 : 1;
}
