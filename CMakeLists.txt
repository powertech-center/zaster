# Copyright (c) 2026-present, PowerTech.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.15)
project(zaster C)

# =============================================================================
# Options
# =============================================================================

option(ZASTER_LINK_ZSTD "Link libzstd (prebuilt or from sources)" ON)

# ZASTER_TARGET — user-facing variable. Can be set by the parent project or
# passed via -DZASTER_TARGET=... on the command line. Not modified here.

# ZASTER_TARGET_RESOLVED — internal cache of the resolved target triple.
# Set from ZASTER_TARGET when provided, otherwise auto-detected and cached
# so that compiler detection is skipped on subsequent configure runs.
set(ZASTER_TARGET_RESOLVED "" CACHE INTERNAL "Resolved target triple")

# =============================================================================
# Determine target triple
# =============================================================================

if(NOT "${ZASTER_TARGET}" STREQUAL "")
    # User provided an explicit value — use it and update the cache.
    if(NOT "${ZASTER_TARGET_RESOLVED}" STREQUAL "${ZASTER_TARGET}")
        set(ZASTER_TARGET_RESOLVED "${ZASTER_TARGET}" CACHE INTERNAL "Resolved target triple" FORCE)
    endif()
    message(STATUS "ZASTER_TARGET = ${ZASTER_TARGET_RESOLVED} (user-defined)")
elseif(NOT "${ZASTER_TARGET_RESOLVED}" STREQUAL "")
    # Cached value from a previous run — nothing to do.
    message(STATUS "ZASTER_TARGET = ${ZASTER_TARGET_RESOLVED} (cached)")
else()
    # Nothing cached — auto-detect from the compiler.

    # Prefer compiler target triple (set by toolchain files for cross-compilation)
    if(CMAKE_C_COMPILER_TARGET)
        set(_zaster_target_raw "${CMAKE_C_COMPILER_TARGET}")
    elseif(CMAKE_CXX_COMPILER_TARGET)
        set(_zaster_target_raw "${CMAKE_CXX_COMPILER_TARGET}")
    else()
        # Fall back to asking the compiler directly.
        # MSVC (cl) and clang-cl do not support -dumpmachine; skip them.
        if(CMAKE_C_COMPILER
           AND NOT CMAKE_C_COMPILER_ID STREQUAL "MSVC"
           AND NOT CMAKE_C_COMPILER MATCHES "clang-cl")
            set(_zaster_compiler "${CMAKE_C_COMPILER}")
        elseif(CMAKE_CXX_COMPILER
               AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
               AND NOT CMAKE_CXX_COMPILER MATCHES "clang-cl")
            set(_zaster_compiler "${CMAKE_CXX_COMPILER}")
        else()
            set(_zaster_compiler "")
        endif()

        if(_zaster_compiler)
            execute_process(
                COMMAND ${_zaster_compiler} -dumpmachine
                OUTPUT_VARIABLE _zaster_target_raw
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        else()
            set(_zaster_target_raw "")
        endif()
    endif()

    # Normalize arch — prefer raw triple, fall back to CMAKE_SYSTEM_PROCESSOR
    if(_zaster_target_raw MATCHES "^(x86_64|amd64|EM64T)")
        set(_zaster_arch "x86_64")
    elseif(_zaster_target_raw MATCHES "^(aarch64|arm64|ARM64)")
        set(_zaster_arch "aarch64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|EM64T)")
        set(_zaster_arch "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)")
        set(_zaster_arch "aarch64")
    else()
        set(_zaster_arch "${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    # Normalize OS + ABI — prefer parsing from the raw triple, fall back to CMAKE_SYSTEM_NAME
    if(_zaster_target_raw MATCHES "linux-musl")
        set(_zaster_os "linux-musl")
    elseif(_zaster_target_raw MATCHES "linux")
        set(_zaster_os "linux-gnu")
    elseif(_zaster_target_raw MATCHES "apple-darwin" OR _zaster_target_raw MATCHES "darwin")
        set(_zaster_os "apple-darwin")
    elseif(_zaster_target_raw MATCHES "windows-msvc" OR _zaster_target_raw MATCHES "msvc")
        set(_zaster_os "windows-msvc")
    elseif(_zaster_target_raw MATCHES "windows" OR _zaster_target_raw MATCHES "mingw")
        set(_zaster_os "windows-gnu")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(_zaster_os "linux-gnu")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(_zaster_os "apple-darwin")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_C_COMPILER_ID STREQUAL "MSVC"
           OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
           OR CMAKE_C_COMPILER MATCHES "clang-cl"
           OR CMAKE_CXX_COMPILER MATCHES "clang-cl")
            set(_zaster_os "windows-msvc")
        else()
            set(_zaster_os "windows-gnu")
        endif()
    else()
        set(_zaster_os "${CMAKE_SYSTEM_NAME}")
    endif()

    set(ZASTER_TARGET_RESOLVED "${_zaster_arch}-${_zaster_os}" CACHE INTERNAL "Resolved target triple" FORCE)
    message(STATUS "ZASTER_TARGET = ${ZASTER_TARGET_RESOLVED} (auto-detected)")
endif()

# Extract arch from the resolved target for source-build conditionals
if(ZASTER_TARGET_RESOLVED MATCHES "^x86_64")
    set(_zaster_arch "x86_64")
elseif(ZASTER_TARGET_RESOLVED MATCHES "^aarch64")
    set(_zaster_arch "aarch64")
else()
    set(_zaster_arch "")
endif()

# =============================================================================
# libzstd
# =============================================================================

if(ZASTER_LINK_ZSTD)
    set(_zstd_prebuilt "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/${ZASTER_TARGET_RESOLVED}/libzstd.a")

    if(EXISTS "${_zstd_prebuilt}")
        message(STATUS "libzstd: using prebuilt ${_zstd_prebuilt}")
        add_library(zstd_impl STATIC IMPORTED GLOBAL)
        set_target_properties(zstd_impl PROPERTIES
            IMPORTED_LOCATION "${_zstd_prebuilt}"
        )
    else()
        message(STATUS "libzstd: prebuilt not found, building from sources")

        file(GLOB _zstd_sources
            "${CMAKE_CURRENT_SOURCE_DIR}/zstd/lib/common/*.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/zstd/lib/compress/*.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/zstd/lib/decompress/*.c"
        )

        # huf_decompress_amd64.S — x86_64 only; requires ASM language
        if(_zaster_arch STREQUAL "x86_64")
            enable_language(ASM)
            list(APPEND _zstd_sources
                "${CMAKE_CURRENT_SOURCE_DIR}/zstd/lib/decompress/huf_decompress_amd64.S"
            )
        endif()

        add_library(zstd_impl STATIC ${_zstd_sources})
        target_include_directories(zstd_impl PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/zstd/lib"
            "${CMAKE_CURRENT_SOURCE_DIR}/zstd/lib/common"
        )
        set_target_properties(zstd_impl PROPERTIES
            C_STANDARD 11
            POSITION_INDEPENDENT_CODE ON
        )
    endif()
endif()

# =============================================================================
# libzaster
# =============================================================================

set(_zaster_prebuilt "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/${ZASTER_TARGET_RESOLVED}/libzaster.a")

if(EXISTS "${_zaster_prebuilt}")
    message(STATUS "libzaster: using prebuilt ${_zaster_prebuilt}")
    add_library(zaster_impl STATIC IMPORTED GLOBAL)
    set_target_properties(zaster_impl PROPERTIES
        IMPORTED_LOCATION "${_zaster_prebuilt}"
    )
else()
    message(STATUS "libzaster: prebuilt not found, building from sources")

    file(GLOB _zaster_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.c"
    )

    add_library(zaster_impl STATIC ${_zaster_sources})
    target_include_directories(zaster_impl PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/zstd/lib"
    )
    set_target_properties(zaster_impl PROPERTIES
        C_STANDARD 11
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# =============================================================================
# Interface target: zaster::zaster
# =============================================================================

add_library(zaster INTERFACE)
add_library(zaster::zaster ALIAS zaster)

target_include_directories(zaster INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(zaster INTERFACE zaster_impl)

if(ZASTER_LINK_ZSTD)
    target_link_libraries(zaster INTERFACE zstd_impl)
endif()
