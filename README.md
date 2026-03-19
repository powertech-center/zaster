# Zaster
High-performance zstd/tar.zst bindings for C/C++, Rust, Go, Python and Node.js

## Multi-threading Magic

Zaster supports two modes for tar.zst archives: standard and multi-threaded.

**Standard mode** creates a regular tar.zst archive — one continuous compressed stream. Compatible with any tar/zstd tool. Decompression happens sequentially, one block at a time.

**Multi-threaded mode** splits data into independent frames with special metadata, producing an archive that can be unpacked in parallel — multiple threads working simultaneously on different parts.

The key insight: if blazing-fast extraction is your goal, you need both sides. Create in multi-threaded mode to build the optimized structure, then extract in multi-threaded mode to leverage it. Extracting a standard archive with multi-threading won't give you the same speed boost — the parallel-friendly structure simply isn't there.

## CLI

Zaster ships with a CLI utility that demonstrates its tar.zst capabilities and works as a standalone tool for shell workflows.

```
Usage:
  zaster [OPTIONS] <archive> <directory> [patterns...]

Options:
  -h                        Show this help
  -v                        Show version
  -c[=LEVEL][m[=THREADS]]   Create archive
  -e[m[=THREADS]]           Extract archive
  -l                        List archive contents

Examples:
  zaster -c archive.tar.zst dir/
  zaster -c=11m=4 archive.tar.zst dir/ "*.conf" "*.toml"
  zaster -cm archive.tar.zst dir/ "etc/*"
  zaster -e archive.tar.zst output/
  zaster -em=8 archive.tar.zst output/ "*.log"
  zaster -l archive.tar.zst
```

Compression levels range from 1 to 22 (default: 3). Multi-threading is enabled with the `m` modifier — use `-cm` or `-em` for auto-detection, or specify thread count like `-cm=4`.

## License

See [LICENSE](LICENSE) file for details.
