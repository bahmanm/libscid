# Explanation and Architecture

This section provides understanding-oriented discussions exploring the architectural models, design rationale, memory invariants, and algorithmic trade-offs underpinning the `libscid` Python bindings.

---

## 1. Architectural Foundations

- [C ABI Bridge Architecture](bridge-architecture.md): The two-tier architecture bridging Python and the native C11 ABI, including library discovery and status translation.
- [Immutable Cursor Mental Model](cursor-immutability.md): The design rationale behind immutable cursor navigation, contrast with mutable iterators, and tree mutation semantics.
- [Memory Model and Lifecycles](memory-and-lifecycles.md): Resource ownership across the C/Python boundary, weakref finalisation, and parent reference retention.
- [In-Memory Indexing and Filter Algebra](indexing-and-filters.md): Mechanics of in-memory PGN index tables, lazy movetext decoding, and 64-bit word bitset algebra.
- [Event Streams vs Recursive Traversal](event-stream-vs-tree-walk.md): Comparative analysis between generator-based AST event token streams and recursive tree walkers.
