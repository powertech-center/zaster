/*
 * Copyright (c) 2026-present, PowerTech.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ZASTER_HPP
#define ZASTER_HPP

#include "zaster.h"

/* =========================================================================
 * namespace zstd — inline C++ wrappers over the zstd C API
 *
 * Naming: ZSTD_fooBar() → zstd::fooBar()
 *         ZSTD_FooBar   → zstd::FooBar  (types, enums)
 *         ZSTD_FOO_BAR  → zstd::FOO_BAR (constants as constexpr)
 * ========================================================================= */

namespace zstd {

/* -------------------------------------------------------------------------
 * Types (aliases, no overhead)
 * ------------------------------------------------------------------------- */

using CCtx          = ZSTD_CCtx;
using DCtx          = ZSTD_DCtx;
using CStream       = ZSTD_CStream;
using DStream       = ZSTD_DStream;
using CDict         = ZSTD_CDict;
using DDict         = ZSTD_DDict;
using inBuffer      = ZSTD_inBuffer;
using outBuffer     = ZSTD_outBuffer;
using bounds        = ZSTD_bounds;
using cParameter    = ZSTD_cParameter;
using dParameter    = ZSTD_dParameter;
using strategy      = ZSTD_strategy;
using EndDirective  = ZSTD_EndDirective;
using ResetDirective = ZSTD_ResetDirective;
using ErrorCode     = ZSTD_ErrorCode;

/* -------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------- */

static constexpr int    CLEVEL_DEFAULT          = ZSTD_CLEVEL_DEFAULT;
static constexpr unsigned MAGICNUMBER           = ZSTD_MAGICNUMBER;
static constexpr unsigned MAGIC_DICTIONARY      = ZSTD_MAGIC_DICTIONARY;
static constexpr unsigned MAGIC_SKIPPABLE_START = ZSTD_MAGIC_SKIPPABLE_START;
static constexpr unsigned MAGIC_SKIPPABLE_MASK  = ZSTD_MAGIC_SKIPPABLE_MASK;
static constexpr int    BLOCKSIZELOG_MAX        = ZSTD_BLOCKSIZELOG_MAX;
static constexpr int    BLOCKSIZE_MAX           = ZSTD_BLOCKSIZE_MAX;
static constexpr unsigned long long CONTENTSIZE_UNKNOWN = ZSTD_CONTENTSIZE_UNKNOWN;
static constexpr unsigned long long CONTENTSIZE_ERROR   = ZSTD_CONTENTSIZE_ERROR;

/* -------------------------------------------------------------------------
 * Version
 * ------------------------------------------------------------------------- */

ZASTER_INLINE unsigned versionNumber() { return ZSTD_versionNumber(); }
ZASTER_INLINE const char* versionString() { return ZSTD_versionString(); }

/* -------------------------------------------------------------------------
 * Simple API
 * ------------------------------------------------------------------------- */

ZASTER_INLINE size_t compress(void* dst, size_t dstCapacity,
                               const void* src, size_t srcSize,
                               int compressionLevel) {
    return ZSTD_compress(dst, dstCapacity, src, srcSize, compressionLevel);
}

ZASTER_INLINE size_t decompress(void* dst, size_t dstCapacity,
                                 const void* src, size_t compressedSize) {
    return ZSTD_decompress(dst, dstCapacity, src, compressedSize);
}

ZASTER_INLINE unsigned long long getFrameContentSize(const void* src, size_t srcSize) {
    return ZSTD_getFrameContentSize(src, srcSize);
}

ZASTER_INLINE size_t findFrameCompressedSize(const void* src, size_t srcSize) {
    return ZSTD_findFrameCompressedSize(src, srcSize);
}

ZASTER_INLINE size_t compressBound(size_t srcSize) {
    return ZSTD_compressBound(srcSize);
}

/* -------------------------------------------------------------------------
 * Error helpers
 * ------------------------------------------------------------------------- */

ZASTER_INLINE unsigned    isError(size_t result)              { return ZSTD_isError(result); }
ZASTER_INLINE ErrorCode   getErrorCode(size_t result)         { return ZSTD_getErrorCode(result); }
ZASTER_INLINE const char* getErrorName(size_t result)         { return ZSTD_getErrorName(result); }
ZASTER_INLINE int         minCLevel()                         { return ZSTD_minCLevel(); }
ZASTER_INLINE int         maxCLevel()                         { return ZSTD_maxCLevel(); }
ZASTER_INLINE int         defaultCLevel()                     { return ZSTD_defaultCLevel(); }

/* -------------------------------------------------------------------------
 * Explicit context — compression
 * ------------------------------------------------------------------------- */

ZASTER_INLINE CCtx*  createCCtx()                             { return ZSTD_createCCtx(); }
ZASTER_INLINE size_t freeCCtx(CCtx* cctx)                     { return ZSTD_freeCCtx(cctx); }

ZASTER_INLINE size_t compressCCtx(CCtx* cctx,
                                   void* dst, size_t dstCapacity,
                                   const void* src, size_t srcSize,
                                   int compressionLevel) {
    return ZSTD_compressCCtx(cctx, dst, dstCapacity, src, srcSize, compressionLevel);
}

ZASTER_INLINE bounds cParam_getBounds(cParameter param)       { return ZSTD_cParam_getBounds(param); }
ZASTER_INLINE size_t CCtx_setParameter(CCtx* cctx, cParameter param, int value) {
    return ZSTD_CCtx_setParameter(cctx, param, value);
}
ZASTER_INLINE size_t CCtx_setPledgedSrcSize(CCtx* cctx, unsigned long long pledgedSrcSize) {
    return ZSTD_CCtx_setPledgedSrcSize(cctx, pledgedSrcSize);
}
ZASTER_INLINE size_t CCtx_reset(CCtx* cctx, ResetDirective reset) {
    return ZSTD_CCtx_reset(cctx, reset);
}
ZASTER_INLINE size_t compress2(CCtx* cctx,
                                void* dst, size_t dstCapacity,
                                const void* src, size_t srcSize) {
    return ZSTD_compress2(cctx, dst, dstCapacity, src, srcSize);
}

/* -------------------------------------------------------------------------
 * Explicit context — decompression
 * ------------------------------------------------------------------------- */

ZASTER_INLINE DCtx*  createDCtx()                             { return ZSTD_createDCtx(); }
ZASTER_INLINE size_t freeDCtx(DCtx* dctx)                     { return ZSTD_freeDCtx(dctx); }

ZASTER_INLINE size_t decompressDCtx(DCtx* dctx,
                                     void* dst, size_t dstCapacity,
                                     const void* src, size_t srcSize) {
    return ZSTD_decompressDCtx(dctx, dst, dstCapacity, src, srcSize);
}

ZASTER_INLINE bounds dParam_getBounds(dParameter param)       { return ZSTD_dParam_getBounds(param); }
ZASTER_INLINE size_t DCtx_setParameter(DCtx* dctx, dParameter param, int value) {
    return ZSTD_DCtx_setParameter(dctx, param, value);
}
ZASTER_INLINE size_t DCtx_reset(DCtx* dctx, ResetDirective reset) {
    return ZSTD_DCtx_reset(dctx, reset);
}

/* -------------------------------------------------------------------------
 * Streaming — compression
 * ------------------------------------------------------------------------- */

ZASTER_INLINE CStream* createCStream()                        { return ZSTD_createCStream(); }
ZASTER_INLINE size_t   freeCStream(CStream* zcs)              { return ZSTD_freeCStream(zcs); }
ZASTER_INLINE size_t   CStreamInSize()                        { return ZSTD_CStreamInSize(); }
ZASTER_INLINE size_t   CStreamOutSize()                       { return ZSTD_CStreamOutSize(); }

ZASTER_INLINE size_t compressStream2(CCtx* cctx,
                                      outBuffer* output,
                                      inBuffer* input,
                                      EndDirective endOp) {
    return ZSTD_compressStream2(cctx, output, input, endOp);
}
ZASTER_INLINE size_t initCStream(CStream* zcs, int compressionLevel) {
    return ZSTD_initCStream(zcs, compressionLevel);
}
ZASTER_INLINE size_t compressStream(CStream* zcs, outBuffer* output, inBuffer* input) {
    return ZSTD_compressStream(zcs, output, input);
}
ZASTER_INLINE size_t flushStream(CStream* zcs, outBuffer* output) {
    return ZSTD_flushStream(zcs, output);
}
ZASTER_INLINE size_t endStream(CStream* zcs, outBuffer* output) {
    return ZSTD_endStream(zcs, output);
}

/* -------------------------------------------------------------------------
 * Streaming — decompression
 * ------------------------------------------------------------------------- */

ZASTER_INLINE DStream* createDStream()                        { return ZSTD_createDStream(); }
ZASTER_INLINE size_t   freeDStream(DStream* zds)              { return ZSTD_freeDStream(zds); }
ZASTER_INLINE size_t   DStreamInSize()                        { return ZSTD_DStreamInSize(); }
ZASTER_INLINE size_t   DStreamOutSize()                       { return ZSTD_DStreamOutSize(); }

ZASTER_INLINE size_t initDStream(DStream* zds)                { return ZSTD_initDStream(zds); }
ZASTER_INLINE size_t decompressStream(DStream* zds, outBuffer* output, inBuffer* input) {
    return ZSTD_decompressStream(zds, output, input);
}

/* -------------------------------------------------------------------------
 * Dictionary API
 * ------------------------------------------------------------------------- */

ZASTER_INLINE size_t compress_usingDict(CCtx* ctx,
                                         void* dst, size_t dstCapacity,
                                         const void* src, size_t srcSize,
                                         const void* dict, size_t dictSize,
                                         int compressionLevel) {
    return ZSTD_compress_usingDict(ctx, dst, dstCapacity, src, srcSize, dict, dictSize, compressionLevel);
}
ZASTER_INLINE size_t decompress_usingDict(DCtx* dctx,
                                           void* dst, size_t dstCapacity,
                                           const void* src, size_t srcSize,
                                           const void* dict, size_t dictSize) {
    return ZSTD_decompress_usingDict(dctx, dst, dstCapacity, src, srcSize, dict, dictSize);
}

ZASTER_INLINE CDict* createCDict(const void* dictBuffer, size_t dictSize, int compressionLevel) {
    return ZSTD_createCDict(dictBuffer, dictSize, compressionLevel);
}
ZASTER_INLINE size_t freeCDict(CDict* cdict)                  { return ZSTD_freeCDict(cdict); }
ZASTER_INLINE size_t compress_usingCDict(CCtx* cctx,
                                          void* dst, size_t dstCapacity,
                                          const void* src, size_t srcSize,
                                          const CDict* cdict) {
    return ZSTD_compress_usingCDict(cctx, dst, dstCapacity, src, srcSize, cdict);
}

ZASTER_INLINE DDict* createDDict(const void* dictBuffer, size_t dictSize) {
    return ZSTD_createDDict(dictBuffer, dictSize);
}
ZASTER_INLINE size_t freeDDict(DDict* ddict)                  { return ZSTD_freeDDict(ddict); }
ZASTER_INLINE size_t decompress_usingDDict(DCtx* dctx,
                                            void* dst, size_t dstCapacity,
                                            const void* src, size_t srcSize,
                                            const DDict* ddict) {
    return ZSTD_decompress_usingDDict(dctx, dst, dstCapacity, src, srcSize, ddict);
}

ZASTER_INLINE unsigned getDictID_fromDict(const void* dict, size_t dictSize) {
    return ZSTD_getDictID_fromDict(dict, dictSize);
}
ZASTER_INLINE unsigned getDictID_fromCDict(const CDict* cdict) { return ZSTD_getDictID_fromCDict(cdict); }
ZASTER_INLINE unsigned getDictID_fromDDict(const DDict* ddict) { return ZSTD_getDictID_fromDDict(ddict); }
ZASTER_INLINE unsigned getDictID_fromFrame(const void* src, size_t srcSize) {
    return ZSTD_getDictID_fromFrame(src, srcSize);
}

ZASTER_INLINE size_t CCtx_loadDictionary(CCtx* cctx, const void* dict, size_t dictSize) {
    return ZSTD_CCtx_loadDictionary(cctx, dict, dictSize);
}
ZASTER_INLINE size_t CCtx_refCDict(CCtx* cctx, const CDict* cdict) {
    return ZSTD_CCtx_refCDict(cctx, cdict);
}
ZASTER_INLINE size_t DCtx_loadDictionary(DCtx* dctx, const void* dict, size_t dictSize) {
    return ZSTD_DCtx_loadDictionary(dctx, dict, dictSize);
}
ZASTER_INLINE size_t DCtx_refDDict(DCtx* dctx, const DDict* ddict) {
    return ZSTD_DCtx_refDDict(dctx, ddict);
}

/* -------------------------------------------------------------------------
 * sizeof helpers
 * ------------------------------------------------------------------------- */

ZASTER_INLINE size_t sizeof_CCtx(const CCtx* cctx)           { return ZSTD_sizeof_CCtx(cctx); }
ZASTER_INLINE size_t sizeof_DCtx(const DCtx* dctx)           { return ZSTD_sizeof_DCtx(dctx); }
ZASTER_INLINE size_t sizeof_CStream(const CStream* zcs)       { return ZSTD_sizeof_CStream(zcs); }
ZASTER_INLINE size_t sizeof_DStream(const DStream* zds)       { return ZSTD_sizeof_DStream(zds); }
ZASTER_INLINE size_t sizeof_CDict(const CDict* cdict)         { return ZSTD_sizeof_CDict(cdict); }
ZASTER_INLINE size_t sizeof_DDict(const DDict* ddict)         { return ZSTD_sizeof_DDict(ddict); }

} /* namespace zstd */

/* =========================================================================
 * namespace zaster — inline C++ wrappers over the libzaster C API
 *
 * Naming: zaster_foo_bar() → zaster::foo_bar()
 *         ZasterFooBar     → zaster::FooBar  (types, structs)
 *         ZASTER_FOO_BAR   → zaster::FOO_BAR (constants as constexpr)
 * ========================================================================= */

namespace zaster {

/* =========================================================================
 * namespace zaster::memory
 * ========================================================================= */

namespace memory {

/* -------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------- */

static constexpr size_t PAGE_SIZE            = ZASTER_PAGE_SIZE;
static constexpr size_t SEGMENT_FOOTER_SIZE  = ZASTER_SEGMENT_FOOTER_SIZE;

/* -------------------------------------------------------------------------
 * Page allocation
 * ------------------------------------------------------------------------- */

ZASTER_INLINE void*      alloc_pages(size_t count)            { return zaster_pages_alloc(count); }
ZASTER_INLINE ZasterBool free_pages(void* ptr, size_t count)  { return zaster_pages_free(ptr, count); }
ZASTER_INLINE size_t     size_to_pages(size_t size)           { return zaster_size_to_pages(size); }

/* -------------------------------------------------------------------------
 * Segment
 * ------------------------------------------------------------------------- */

using Segment = ZasterSegment;

ZASTER_INLINE Segment*   segment_new(size_t data_size, Segment* prev) {
    return zaster_segment_new(data_size, prev);
}
ZASTER_INLINE ZasterBool segment_free(Segment* seg, ZasterBool recursive) {
    return zaster_segment_free(seg, recursive);
}

/* -------------------------------------------------------------------------
 * Allocator — RAII wrapper over ZasterAllocator
 *
 * Inherits ZasterAllocator layout exactly (no extra fields).
 * Binary-compatible with the C struct — can be passed to C functions as-is.
 * ------------------------------------------------------------------------- */

struct Allocator : ZasterAllocator {
    ZASTER_ALWAYS_INLINE Allocator()  { zaster_allocator_init(this); }
    ZASTER_ALWAYS_INLINE ~Allocator() { zaster_allocator_free(this); }

    /* Non-copyable: copying a bump allocator makes no sense. */
    Allocator(const Allocator&)            = delete;
    Allocator& operator=(const Allocator&) = delete;

    ZASTER_ALWAYS_INLINE uint8_t* reserve(size_t size) { return zaster_reserve(this, size); }
    ZASTER_ALWAYS_INLINE uint8_t* alloc(size_t size)   { return zaster_alloc(this, size); }
};

} /* namespace memory */

/* =========================================================================
 * namespace zaster::tar
 * ========================================================================= */

namespace tar {

/* -------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------- */

static constexpr int BLOCK_SIZE = ZASTER_TAR_BLOCK_SIZE;

/* Entry type flags */
static constexpr char TYPE_FILE         = ZASTER_TAR_TYPE_FILE;
static constexpr char TYPE_HARD_LINK    = ZASTER_TAR_TYPE_HARD_LINK;
static constexpr char TYPE_SYMLINK      = ZASTER_TAR_TYPE_SYMLINK;
static constexpr char TYPE_CHAR_DEVICE  = ZASTER_TAR_TYPE_CHAR_DEVICE;
static constexpr char TYPE_BLOCK_DEVICE = ZASTER_TAR_TYPE_BLOCK_DEVICE;
static constexpr char TYPE_DIRECTORY    = ZASTER_TAR_TYPE_DIRECTORY;
static constexpr char TYPE_FIFO         = ZASTER_TAR_TYPE_FIFO;
static constexpr char TYPE_GNU_LONGNAME = ZASTER_TAR_TYPE_GNU_LONGNAME;
static constexpr char TYPE_GNU_LONGLINK = ZASTER_TAR_TYPE_GNU_LONGLINK;
static constexpr char TYPE_PAX_EXTENDED = ZASTER_TAR_TYPE_PAX_EXTENDED;
static constexpr char TYPE_PAX_GLOBAL   = ZASTER_TAR_TYPE_PAX_GLOBAL;

/* -------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------- */

using Header = ZasterTarHeader;

} /* namespace tar */

} /* namespace zaster */

#endif /* ZASTER_HPP */
