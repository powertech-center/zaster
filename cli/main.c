/*
 * Copyright (c) 2026-present, PowerTech.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Version
 * ========================================================================= */

#define ZASTER_VERSION "0.1.0"

/* =========================================================================
 * Operation types
 * ========================================================================= */

typedef enum {
    OP_HELP,
    OP_VERSION,
    OP_CREATE,
    OP_EXTRACT,
    OP_LIST
} ZasterOp;

typedef struct {
    ZasterOp    op;
    int         level;      /* compression level (1-22, default 3) */
    int         threads;    /* thread count (-1 = not set, 0 = auto) */
    const char* archive;
    const char* directory;
    int         pattern_count;
    const char** patterns;
} ZasterArgs;

/* =========================================================================
 * Help
 * ========================================================================= */

static void print_help(void) {
    fprintf(stdout,
        "Zaster is a high-performance tar.zst archiver\n"
        "\n"
        "Usage:\n"
        "  zaster [OPTIONS] <archive> <directory> [patterns...]\n"
        "\n"
        "Options:\n"
        "  -h                        Show this help\n"
        "  -v                        Show version\n"
        "  -c[=LEVEL][m[=THREADS]]   Create archive\n"
        "  -e[m[=THREADS]]           Extract archive\n"
        "  -l                        List archive contents\n"
        "\n"
        "Examples:\n"
        "  zaster -c archive.tar.zst dir/\n"
        "  zaster -c=11m=4 archive.tar.zst dir/\n"
        "  zaster -e archive.tar.zst output/\n"
        "  zaster -l archive.tar.zst\n"
    );
}

/* =========================================================================
 * Argument parsing helpers
 *
 * Flag format:
 *   -c[=LEVEL][m[=THREADS]]   create
 *   -e[m[=THREADS]]           extract
 *   -l                        list
 *
 * Level and threads are parsed from a compact suffix without spaces:
 *   -c          -> level=3, threads=not set
 *   -c=11       -> level=11, threads=not set
 *   -cm         -> level=3, threads=auto (0)
 *   -c=11m=4    -> level=11, threads=4
 *   -em=8       -> threads=8
 * ========================================================================= */

/**
 * Parse "=N" from `s`, store value in `out`.
 * Returns pointer past the consumed characters, or NULL on error.
 * If `s` does not start with '=', `out` is left unchanged and `s` is returned.
 */
static const char* parse_eq_int(const char* s, int* out, const char* name) {
    if (*s != '=') return s;
    s++;
    if (*s < '0' || *s > '9') {
        fprintf(stderr, "Error: invalid %s value\n", name);
        return NULL;
    }
    int val = 0;
    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        s++;
    }
    *out = val;
    return s;
}

/**
 * Parse thread suffix: "m" or "m=N".
 * Returns pointer past consumed characters, or NULL on error.
 */
static const char* parse_threads_suffix(const char* s, int* threads) {
    if (*s != 'm') return s;
    s++;
    *threads = 0; /* auto */
    return parse_eq_int(s, threads, "thread count");
}

/**
 * Parse create flag suffix: [=LEVEL][m[=THREADS]]
 */
static int parse_create_flag(const char* suffix, int* level, int* threads) {
    *level   = 3;
    *threads = -1;

    const char* s = suffix;
    s = parse_eq_int(s, level, "compression level");
    if (!s) return 0;

    if (*level < 1 || *level > 22) {
        fprintf(stderr, "Error: compression level must be 1-22, got %d\n", *level);
        return 0;
    }

    s = parse_threads_suffix(s, threads);
    if (!s) return 0;

    if (*s != '\0') {
        fprintf(stderr, "Error: unexpected characters in create flag: %s\n", s);
        return 0;
    }
    return 1;
}

/**
 * Parse extract flag suffix: [m[=THREADS]]
 */
static int parse_extract_flag(const char* suffix, int* threads) {
    *threads = -1;

    const char* s = suffix;
    s = parse_threads_suffix(s, threads);
    if (!s) return 0;

    if (*s != '\0') {
        fprintf(stderr, "Error: unexpected characters in extract flag: %s\n", s);
        return 0;
    }
    return 1;
}

/* =========================================================================
 * Main argument parser
 * ========================================================================= */

static int parse_args(int argc, const char** argv, ZasterArgs* args) {
    memset(args, 0, sizeof(*args));
    args->level   = 3;
    args->threads = -1;

    if (argc < 2) {
        args->op = OP_HELP;
        return 1;
    }

    const char* flag = argv[1];

    if (strcmp(flag, "-h") == 0) {
        args->op = OP_HELP;
        return 1;
    }
    if (strcmp(flag, "-v") == 0) {
        args->op = OP_VERSION;
        return 1;
    }

    /* -c[=LEVEL][m[=THREADS]] */
    if (flag[0] == '-' && flag[1] == 'c') {
        args->op = OP_CREATE;
        if (!parse_create_flag(flag + 2, &args->level, &args->threads))
            return 0;
        if (argc < 4) {
            fprintf(stderr, "Error: create requires: archive directory [patterns...]\n");
            return 0;
        }
        args->archive       = argv[2];
        args->directory      = argv[3];
        args->patterns       = argv + 4;
        args->pattern_count  = argc - 4;
        return 1;
    }

    /* -e[m[=THREADS]] */
    if (flag[0] == '-' && flag[1] == 'e') {
        args->op = OP_EXTRACT;
        if (!parse_extract_flag(flag + 2, &args->threads))
            return 0;
        if (argc < 4) {
            fprintf(stderr, "Error: extract requires: archive directory [patterns...]\n");
            return 0;
        }
        args->archive       = argv[2];
        args->directory      = argv[3];
        args->patterns       = argv + 4;
        args->pattern_count  = argc - 4;
        return 1;
    }

    /* -l */
    if (strcmp(flag, "-l") == 0) {
        args->op = OP_LIST;
        if (argc != 3) {
            fprintf(stderr, "Error: list requires: archive\n");
            return 0;
        }
        args->archive = argv[2];
        return 1;
    }

    if (flag[0] == '-') {
        fprintf(stderr, "Error: unknown option: %s\n", flag);
    } else {
        fprintf(stderr, "Error: missing option. Use -h for help.\n");
    }
    return 0;
}

/* =========================================================================
 * Entry point
 * ========================================================================= */

int main(int argc, const char** argv) {
    ZasterArgs args;
    if (!parse_args(argc, argv, &args)) {
        fprintf(stderr, "\n");
        print_help();
        return 1;
    }

    switch (args.op) {
    case OP_HELP:
        print_help();
        break;

    case OP_VERSION:
        printf("zaster %s\n", ZASTER_VERSION);
        break;

    case OP_CREATE:
        printf("Create: level=%d threads=%d archive=%s dir=%s",
               args.level, args.threads, args.archive, args.directory);
        for (int i = 0; i < args.pattern_count; i++)
            printf(" %s", args.patterns[i]);
        printf("\n");
        /* TODO: call zaster functions */
        break;

    case OP_EXTRACT:
        printf("Extract: threads=%d archive=%s dir=%s",
               args.threads, args.archive, args.directory);
        for (int i = 0; i < args.pattern_count; i++)
            printf(" %s", args.patterns[i]);
        printf("\n");
        /* TODO: call zaster functions */
        break;

    case OP_LIST:
        printf("List: archive=%s\n", args.archive);
        /* TODO: call zaster functions */
        break;
    }

    return 0;
}
