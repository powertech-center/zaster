/*
 * Copyright (c) 2026-present, PowerTech.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ZASTER_TAR_H
#define ZASTER_TAR_H

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * TAR format constants and structures
 *
 * Supports:
 *   - UStar format (POSIX.1-1988)
 *   - GNU longname/longlink extensions (read/write)
 *   - PAX extended headers (read only)
 * ========================================================================= */

/** Block size for tar format (all headers and data aligned to this). */
#define ZASTER_TAR_BLOCK_SIZE 512

/* -------------------------------------------------------------------------
 * UStar magic
 * ------------------------------------------------------------------------- */

/** UStar magic bytes at header offset 257: "ustar\0" */
#define ZASTER_TAR_USTAR_MAGIC  "ustar"
/** UStar version at header offset 263: "00" */
#define ZASTER_TAR_USTAR_VERSION "00"

/* -------------------------------------------------------------------------
 * Entry type flags (typeflag field at header offset 156)
 * ------------------------------------------------------------------------- */

#define ZASTER_TAR_TYPE_FILE         '0'  /**< Regular file */
#define ZASTER_TAR_TYPE_HARD_LINK    '1'  /**< Hard link */
#define ZASTER_TAR_TYPE_SYMLINK      '2'  /**< Symbolic link */
#define ZASTER_TAR_TYPE_CHAR_DEVICE  '3'  /**< Character device */
#define ZASTER_TAR_TYPE_BLOCK_DEVICE '4'  /**< Block device */
#define ZASTER_TAR_TYPE_DIRECTORY    '5'  /**< Directory */
#define ZASTER_TAR_TYPE_FIFO         '6'  /**< FIFO/pipe */

/* GNU extensions */
#define ZASTER_TAR_TYPE_GNU_LONGNAME 'L'  /**< Long filename (>255 chars) */
#define ZASTER_TAR_TYPE_GNU_LONGLINK 'K'  /**< Long linkname (>255 chars) */

/* PAX extensions (read only) */
#define ZASTER_TAR_TYPE_PAX_EXTENDED 'x'  /**< PAX extended header */
#define ZASTER_TAR_TYPE_PAX_GLOBAL   'g'  /**< PAX global header */

/* -------------------------------------------------------------------------
 * Raw TAR header (512 bytes)
 *
 * Layout follows UStar format specification.
 * All numeric fields are octal ASCII strings, NUL-terminated.
 * ------------------------------------------------------------------------- */

typedef struct ZasterTarHeader {
    uint8_t name[100];      /**< File name */
    uint8_t mode[8];        /**< File mode (octal) */
    uint8_t uid[8];         /**< User ID (octal) */
    uint8_t gid[8];         /**< Group ID (octal) */
    uint8_t size[12];       /**< File size in bytes (octal) */
    uint8_t mtime[12];      /**< Modification time (octal, Unix timestamp) */
    uint8_t checksum[8];    /**< Header checksum (octal) */
    uint8_t typeflag;       /**< Entry type */
    uint8_t linkname[100];  /**< Link target name */

    /* UStar extension (bytes 257-511) */
    uint8_t magic[6];       /**< "ustar\0" */
    uint8_t version[2];     /**< "00" */
    uint8_t uname[32];      /**< User name */
    uint8_t gname[32];      /**< Group name */
    uint8_t devmajor[8];    /**< Device major number (octal) */
    uint8_t devminor[8];    /**< Device minor number (octal) */
    uint8_t prefix[155];    /**< Path prefix (for names >100 chars) */
    uint8_t padding[12];    /**< Padding to 512 bytes */
} ZasterTarHeader;

ZASTER_STATIC_ASSERT(sizeof(ZasterTarHeader) == ZASTER_TAR_BLOCK_SIZE,
    "ZasterTarHeader must be exactly 512 bytes");

#ifdef __cplusplus
}
#endif

#endif /* ZASTER_TAR_H */
