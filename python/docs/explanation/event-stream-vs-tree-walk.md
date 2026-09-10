# Event Streams vs Recursive Traversal

This document contrasts the two primary paradigms provided by `libscid` for navigating game move trees: recursive cursor traversal and generator-based event token streaming.

---

## 1. Paradigm Comparison

```
┌────────────────────────────────────────────────────────┐
│                   Game Move Tree                       │
│                   1. e4 e5 (1... c5)                   │
└──────────────┬──────────────────────────┬──────────────┘
               │                          │
               ▼                          ▼
┌──────────────────────────────┐ ┌──────────────────────────────┐
│  Cursor Tree Traversal       │ │  Event Token Stream          │
│  - Spatial & hierarchical    │ │  - Linear & temporal         │
│  - Explicit branch descent   │ │  - Flattened token stream    │
│  - Arbitrary seeks & edits   │ │  - Push/yield generator      │
│  - State inspection at node  │ │  - Pattern matching friendly │
└──────────────────────────────┘ └──────────────────────────────┘
```

| Dimension | Cursor Tree Traversal | Event Token Stream (`iter_movetext`) |
| :--- | :--- | :--- |
| **Mental Model** | Spatial pointer moving through a directed graph | Sequential event token bus (SAX-style parser) |
| **Statefulness** | Point-in-time coordinates wrapped in [`Cursor`][libscid.Cursor] | Stateless stream of [`MovetextEvent`][libscid.MovetextEvent] tokens |
| **Variation Handling** | Explicit branching via `enter_variation()` | Automatic indentation tokens (`MovetextLineStart/End`) |
| **Tree Mutation** | Supported (`append_move`, `add_variation`) | Read-only observation |
| **Control Flow** | Driven by client loop and recursion | Driven by generator iteration (`for event in ...`) |
| **Primary Use Cases** | Interactive UI navigation, tree editing, engines | PGN serialisation, formatting, text rendering, metrics |

---

## 2. Cursor Tree Traversal: Explicit Navigation

In cursor navigation, the client application controls navigation decisions explicitly:

```python
cursor = game.create_cursor()
while not cursor.is_line_end:
    cursor = cursor.next()
    # At any node, inspect variations, evaluate legality, or mutate tree:
    for v_idx in range(cursor.variation_count):
        branch = cursor.enter_variation(v_idx)
        # Process branch...
```

### Strengths
- Random and Bidirectional Access: Jump back to game start, step backwards with `cursor.previous()`, or jump to line ends.
- Board Analysis at Every Ply: Directly query [`cursor.position`][libscid.Cursor.position] for FEN, legal move bitboards, check status, or tournament draw claims.
- Tree Editing: Add moves, delete lines, and update comments in place.

### Weaknesses
- Boilerplate for Full Tree Walking: Serialising or rendering an entire move tree requires recursive functions or explicit stacks to handle arbitrary variation nesting.

---

## 3. Event Token Streaming: Declarative Flattening

The [`iter_movetext()`][libscid.iter_movetext] generator flattens the hierarchical move tree into a sequential token stream:

```python
for event in game.iter_movetext():
    match event:
        case libscid.MovetextLineStart(variation_depth=depth):
            ...
        case libscid.MovetextMove(san=san, comment=cmt):
            ...
        case libscid.MovetextLineEnd():
            ...
```

### Strengths
- Zero Recursion Overhead: The caller does not need to manage call stacks or tree pointers.
- Structural Pattern Matching: Integrates directly with Python 3.10+ `match/case` syntax.
- Streaming Throughput: Generates tokens lazily on demand without constructing intermediate tree wrappers.

### Weaknesses
- Read-Only: Streams cannot be used to edit moves or append variations.
- No Backtracking: Once an event is consumed, the stream cannot be rewound without creating a new generator.

---

## 4. Selecting the Right Pattern

- Choose Cursor Traversal when:
  - Building interactive chess boards or desktop/web UIs where the user steps through moves forward and backward.
  - Generating candidate move lines or validating game legality in chess engines.
  - Mutating the move tree (editing comments, adding variations, deleting moves).
  - Checking draw conditions with [`Arbiter`][libscid.Arbiter] at specific plies.
- Choose Event Streaming when:
  - Writing custom PGN, HTML, or LaTeX chess book export formatters.
  - Extracting a flat sequence of mainline moves.
  - Computing game-level textual statistics (e.g. counting total commentary words or NAG frequencies).
