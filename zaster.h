/*
 * Copyright (c) 2026-present, PowerTech.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ZASTER_H
#define ZASTER_H

/* =========================================================================
 * Zaster public C API
 *
 * Aggregates all public headers:
 *   - zstd public API (zstd.h, zstd_errors.h)
 *   - libzaster modules (memory, tar, fs)
 *
 * Usage: #include "zaster.h"
 * ========================================================================= */

/* --- zstd ---------------------------------------------------------------- */
#include "zstd/lib/zstd.h"
#include "zstd/lib/zstd_errors.h"

/* --- libzaster ----------------------------------------------------------- */
#include "lib/defs.h"
#include "lib/memory.h"
#include "lib/tar.h"
#include "lib/fs.h"

#endif /* ZASTER_H */
