---
inclusion: manual
---

# Правила портирования публичного API

Описывает как C-заголовки libzaster и zstd отображаются в публичный API для каждого языка.

## zaster.h — публичный C API

Файл в корне репозитория. Агрегатор заголовков, ничего лишнего.

Порядок включений:
1. `zstd/lib/zstd.h` и `zstd/lib/zstd_errors.h`
2. `lib/defs.h`
3. Модули libzaster в порядке зависимостей: `memory.h`, `tar.h`, `fs.h`

При добавлении нового модуля в `lib/` — добавить `#include "lib/<module>.h"` в `zaster.h`.

## zaster.hpp — C++ namespace wrapper

Включает `zaster.h`, затем объявляет два namespace.

### namespace zstd::

Тонкая обёртка над C API zstd. Все функции — `ZASTER_INLINE`.

Правила именования:
- `ZSTD_fooBar(...)` → `zstd::fooBar(...)`
- `ZSTD_FooBar` (тип/struct) → `using FooBar = ZSTD_FooBar`
- `ZSTD_FOO_BAR` (константа) → `static constexpr ... FOO_BAR = ZSTD_FOO_BAR`

Deprecated-функции zstd (`ZSTD_DEPRECATED`) в namespace не включаем.

### namespace zaster::

Отражает структуру `lib/`. Каждый модуль — свой вложенный namespace.

| Модуль | Namespace |
|--------|-----------|
| `lib/memory.h` | `zaster::memory::` |
| `lib/tar.h` | `zaster::tar::` |
| `lib/fs.h` | `zaster::fs::` |

Правила именования:
- `zaster_foo_bar(...)` → `zaster::<module>::foo_bar(...)`
- `ZasterFooBar` (тип/struct) → `using FooBar = ZasterFooBar`
- `ZASTER_FOO_BAR` (константа) → `static constexpr ... FOO_BAR = ZASTER_FOO_BAR`
- Префикс модуля убирается: `ZASTER_TAR_BLOCK_SIZE` → `zaster::tar::BLOCK_SIZE`

Все функции-обёртки — `ZASTER_INLINE`, тело — прямой вызов C-функции.

#### Классы с RAII

Если C-тип требует init/free — делаем struct, наследующийся от C-типа:
- Нет собственных полей (бинарная совместимость с C)
- Конструктор по умолчанию вызывает `_init`
- Деструктор вызывает `_free`
- Конструктор копирования и оператор присваивания — `= delete`
- Методы — `ZASTER_INLINE`, прямой вызов C-функций

Пример: `zaster::memory::Allocator` над `ZasterAllocator`.

## Go

*(заполнить при реализации)*

Правила `#cgo` директив, маппинг типов, обработка ошибок.

## Rust

*(заполнить при реализации)*

`bindgen` или ручные `extern "C"` блоки, unsafe-обёртки, safe API.

## Node.js / Python

*(заполнить при реализации)*
