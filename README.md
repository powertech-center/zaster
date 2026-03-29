# Zaster

> **Status: Work in Progress** — The library is under active development and is not yet ready for production use. APIs may change without notice.

High-performance zstd/tar.zst bindings for C, C++, C#, Rust, Go, Python and Node.js.

Zaster provides prebuilt static libraries for 10 target platforms, a unified C/C++ API, and idiomatic bindings for each supported language — all from a single repository.

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

---

## C

Include `zaster.h` and link the prebuilt static libraries for your platform.

### CMake

```cmake
add_subdirectory(vendor/zaster)
target_link_libraries(myapp PRIVATE zaster::zaster)
```

To use your own zstd build:

```cmake
set(ZASTER_LINK_ZSTD OFF)
add_subdirectory(vendor/zaster)
target_link_libraries(myapp PRIVATE zaster::zaster your_zstd_target)
```

### Meson

> Not yet implemented.

```meson
zaster_dep = subproject('zaster').get_variable('zaster_dep')
executable('myapp', 'main.c', dependencies: [zaster_dep])
```

### Make

> Not yet implemented.

```makefile
include vendor/zaster/zaster.mk

myapp: main.c
	$(CC) -I vendor/zaster -o $@ $< $(ZASTER_LDFLAGS)
```

### pkg-config

> Not yet implemented.

```bash
cc -o myapp main.c $(pkg-config --cflags --libs zaster)
```

### Example

```c
#include "zaster.h"

int main(void) {
    unsigned version = ZSTD_versionNumber();

    ZasterAllocator alloc;
    zaster_allocator_init(&alloc);

    void* buf = zaster_alloc(&alloc, 1024);
    // ... use buf ...

    zaster_allocator_free(&alloc);
    return 0;
}
```

---

## C++

Include `zaster.hpp` for the C++ API with `namespace zstd::` and `namespace zaster::` wrappers. Build system integration is the same as C.

### Example

```cpp
#include "zaster.hpp"

int main() {
    auto version = zstd::versionNumber();

    zaster::memory::Allocator alloc;
    auto* buf = alloc.alloc(1024);
    // ... use buf ...
    // Allocator freed automatically via RAII
    return 0;
}
```

---

## Go

> Not yet implemented.

The Go module lives at the repository root due to CGo constraints — Go modules cannot reference files outside the module directory.

### Installation

```bash
go get github.com/nicktretyakov/zaster
```

### Example

```go
package main

import "github.com/nicktretyakov/zaster"

func main() {
    version := zaster.VersionNumber()

    data := []byte("hello, zaster!")
    compressed, err := zaster.Compress(data, 3)
    if err != nil {
        panic(err)
    }

    decompressed, err := zaster.Decompress(compressed)
    if err != nil {
        panic(err)
    }
    _ = decompressed
}
```

---

## Rust

> Not yet implemented.

The Rust crate is located in the `rust/` directory. It uses `build.rs` to link prebuilt static libraries from `prebuilt/`.

### Installation

Add to your `Cargo.toml`:

```toml
[dependencies]
zaster = { git = "https://github.com/nicktretyakov/zaster", tag = "v0.1.0" }
```

### Example

```rust
use zaster;

fn main() {
    let version = zaster::version_number();

    let data = b"hello, zaster!";
    let compressed = zaster::compress(data, 3).unwrap();
    let decompressed = zaster::decompress(&compressed).unwrap();
    assert_eq!(data, &decompressed[..]);
}
```

---

## C#

> Not yet implemented.

The C# binding is located in the `csharp/` directory. It uses P/Invoke to call native functions from dynamically linked libraries.

### Installation

```bash
dotnet add package Zaster
```

### Example

```csharp
using Zaster;

var version = Zaster.VersionNumber();

byte[] data = System.Text.Encoding.UTF8.GetBytes("hello, zaster!");
byte[] compressed = Zaster.Compress(data, level: 3);
byte[] decompressed = Zaster.Decompress(compressed);
```

---

## Python

> Not yet implemented.

The Python binding is located in the `python/` directory. It uses cffi or ctypes to call native functions from shared libraries. Distributed via PyPI as platform-specific wheels.

### Installation

```bash
pip install zaster
```

### Example

```python
import zaster

version = zaster.version_number()

data = b"hello, zaster!"
compressed = zaster.compress(data, level=3)
decompressed = zaster.decompress(compressed)
assert data == decompressed
```

---

## Node.js

> Not yet implemented.

The Node.js binding is located in the `nodejs/` directory. It uses N-API native addons with prebuilt binaries for supported platforms. Distributed via npm.

### Installation

```bash
npm install zaster
```

### Example

```javascript
const zaster = require('zaster');

const version = zaster.versionNumber();

const data = Buffer.from('hello, zaster!');
const compressed = zaster.compress(data, { level: 3 });
const decompressed = zaster.decompress(compressed);
```

---

## Repository Structure

```
/                     Public API and build configurations
  zaster.h            C API
  zaster.hpp          C++ API
  CMakeLists.txt      CMake configuration
  Makefile            Make configuration (not yet implemented)
  meson.build         Meson configuration (not yet implemented)
  zaster.pc.in        pkg-config template (not yet implemented)
  go.mod              Go module (not yet implemented)
zstd/                 Vendored zstd sources
lib/                  libzaster sources
cli/                  CLI utility sources
prebuilt/             Prebuilt static libraries for all targets
dist/                 Release artifacts (in .gitignore)
tests/                C/C++ tests
rust/                 Rust binding (not yet implemented)
csharp/               C# binding (not yet implemented)
python/               Python binding (not yet implemented)
nodejs/               Node.js binding (not yet implemented)
```

## License

See [LICENSE](LICENSE) file for details.
