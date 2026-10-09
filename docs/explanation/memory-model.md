# Memory Management and Buffer Semantics

This document details the ownership semantics, buffer management patterns, and error handling architecture employed across the libscid C ABI.

---

## 1. The Caller-Allocated Buffer Pattern

In foreign-function interface design, memory allocations across library boundaries represent a primary source of memory leaks, heap fragmentation, and allocator incompatibilities. If a library allocates memory internally (e.g. via `malloc`) and hands a pointer to the caller, the caller's runtime must know which allocator to use when releasing it.

To eliminate this vulnerability, the libscid C ABI adopts the caller-allocated buffer pattern for all variable-length text strings and arrays:

- Explicit Buffer Parameters: Text-output functions take a destination buffer, its capacity in bytes, and an output-size pointer. For these functions, `out_size` is required even when querying the size or writing the result.
- Safe Two-Pass Sizing: With a NULL buffer and zero capacity, a text-output function reports the text length (excluding the terminating NUL) in `*out_size` and returns `SCID_ERROR_BUFFER_FULL` when the text is nonempty. Allocate at least that length plus one byte, then call again with a valid size pointer. The second call succeeds only when the capacity includes space for the terminating NUL. On insufficient capacity, the function reports the text length, not the total capacity, in `*out_size`.
- Zero Unnecessary Allocations: Callers with known stack limits (e.g. formatting a standard FEN string or square name) can provide stack-allocated buffers and avoid heap overhead entirely.

```c
/* Example: Two-pass buffer sizing */
size_t required_size = 0;
scid_error err = scid_game_to_pgn(game, NULL, NULL, 0, &required_size);
if (err == SCID_ERROR_BUFFER_FULL)
{
    char* buffer = malloc(required_size + 1); /* Include space for the NUL. */
    if (buffer)
    {
        size_t written_size = 0;
        err = scid_game_to_pgn(game, NULL, buffer, required_size + 1, &written_size);
        if (err == SCID_OK)
        {
            /* buffer is NUL-terminated; written_size excludes the NUL. */
            /* ... process buffer ... */
        }
        free(buffer);
    }
}
```

---

## 2. Status Classification and Return Codes

Every ABI function that can fail returns a `scid_error` enum value. Status codes are partitioned into three categories:

- Success (`SCID_OK = 0`): The operation completed without error or anomaly.
- Warnings (values where `scid_is_warning(status)` returns true): The operation completed, but an anomaly was detected (e.g. `SCID_WARNING_NAME_DATA_LOSS` indicating truncated namebase records in corrupt databases).
- Errors (values where `scid_is_error(status)` returns true): The operation failed (e.g. `SCID_ERROR_BAD_ARG`, `SCID_ERROR_INVALID_FEN`, `SCID_ERROR_BUFFER_FULL`).

Status classification functions are provided as compiled functions rather than preprocessor macros, ensuring robust language binding across foreign runtimes without macro expansion support.

---

## 3. Handle Lifecycles and Safety Invariants

For stateful abstractions such as `scid_position`, `scid_game`, and `scid_database`, lifecycle management is governed by explicit invariants:

- Heap Containment: Internal memory allocations remain strictly inside the C++ subsystem.
- Deterministic Deallocation: Handles must be explicitly released using their dedicated `*_free()` functions.
- NULL-Resilience: All deallocation functions (`scid_position_free`, `scid_game_free`, `scid_game_cursor_free`, `scid_database_free`, etc.) accept `NULL` safely as a no-op, simplifying cleanup blocks and `goto`-style error paths in client C code.
