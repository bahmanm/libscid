# C ABI Design and Encapsulation

This document explains the architectural principles, trade-offs, and design rationale behind the libscid C Application Binary Interface (C ABI).

---

## 1. Rationale for a Pure C ABI

libscid's core engine is implemented in modern C++23. However, the published consumer contract is exposed strictly as a C11 ABI declared in `scid/scid.h`. This choice is driven by three foundational architectural requirements:

- Compiler ABI Independence: C++ lacks a standardised Application Binary Interface across compilers, versions, and standard library implementations (such as `libstdc++` versus `libc++`). Exposing C++ types (e.g. `std::string`, `std::vector`, or virtual method tables) would bind downstream consumers to the exact compiler version and standard library runtime used to build libscid. A pure C ABI ensures binary compatibility across diverse toolchains.
- Foreign Function Interface (FFI) Interoperability: C is the universal language of modern runtime interop. Language runtimes including Python (via `ctypes` or `cffi`), Rust, Go, Java (via JNI/Panama), C#, and Swift bind effortlessly to C symbol names and data types without intermediate C++ translation layers.
- Encapsulation of Legacy Machinery: libscid evolved from Scid and ScidUp, encapsulating over two decades of chess database logic. A strict C ABI creates an impermeable boundary, allowing internal C++ classes, algorithms, and data structures to be refactored, modernised, or replaced without breaking binary compatibility for client applications.

---

## 2. Opaque Handle Pattern

All complex entities in libscid with non-trivial lifecycles or internal state are managed as opaque pointers:

- `scid_position`: Represents complete chess board state and move legality.
- `scid_game`: Encapsulates a complete game including header tags, root position, and move tree.
- `scid_game_cursor`: Represents an immutable navigation position within a game's move tree.
- `scid_game_pgn_options`: Holds configurable options for PGN export formatting.
- `scid_database`: Encapsulates database storage engines (in-memory, native SCID5, or read-only PGN).
- `scid_search_header_criteria`: Builder for filtering games by metadata tags.
- `scid_search_board_criteria`: Builder for filtering games by board and pawn geometry.
- `scid_eco_book`: Loaded opening book for classification.

### Lifecycle Contracts

- Allocation: Handles are created exclusively via dedicated factory functions accepting an output parameter, such as `scid_game_create(..., scid_game** out_game, ...)`.
- Deallocation: Every handle must be released with its corresponding `*_free()` function, such as `scid_game_free(game)`.
- NULL-Safety: All `*_free()` functions treat `NULL` as a valid no-op, preventing double-free crashes during error cleanup paths.
- Heap Independence: Downstream applications never invoke `free()` or `delete` on libscid handles directly; memory is always reclaimed by the library runtime that allocated it.

---

## 3. Value Types versus Opaque Handles

Types representing fundamental chess primitives are exposed as concrete C value types:

- Primitive scalar aliases: `scid_colour`, `scid_square`, `scid_piece`, `scid_nag`, and `scid_error`.
- Small value structs: `scid_movespec` contains small integer fields (`from`, `to`, `promotion`, `is_castling`) and is passed by value or shallow pointer.

This separation ensures that high-frequency calculations (such as iterating through legal moves or checking square contents) incur zero heap allocation overhead, whilst complex state trees remain completely encapsulated.

---

## 4. Cursor Immutability

Navigation within a game's variation tree is mediated by `scid_game_cursor`. Cursors are deliberately immutable:

- Navigational steps (e.g. `scid_game_cursor_next()`, `scid_game_cursor_previous()`, or `scid_game_cursor_variation_enter()`) do not mutate the existing cursor; instead, they output a newly allocated cursor instance.
- This design prevents iterator invalidation when multiple threads or consumer routines inspect or traverse disparate variations of the same game tree simultaneously.
