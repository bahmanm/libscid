# libscid Python Documentation

<img src="assets/img/libscid-16x9.png" alt="libscid Logo" align="left" width="30%" style="max-width: 30%; margin-right: 1.5rem; margin-bottom: 1rem;" />

`libscid` is a high-performance chess library providing modern Python bindings built atop the standalone `libscid` C ABI. It delivers fast game representation, standard-compliant PGN parsing and serialisation, functional navigation cursors, board analysis, tournament arbiter adjudication, and multi-criteria database indexing and search.

<div style="clear: both;"></div>

---

## 1. Documentation Structure

The documentation is organised according to the Diataxis framework:

- [Tutorials](tutorials/quickstart.md): Learning-oriented guided lessons to get up and running with the Python bindings. Start here if you are using `libscid` for the first time.
- [How-To Guides](how-to/index.md): Practical, recipe-oriented guides showing how to accomplish specific tasks including PGN serialisation, variation tree navigation, database searching, and tournament draw adjudication.
- [Explanation](explanation/index.md): Conceptual models, C ABI bridge architecture, memory management lifecycles, and design rationale behind the Python layer.
- [API Reference](reference/index.md): Complete, authoritative technical specification of all public classes, methods, dataclasses, and error types.

---

## 2. Where to Begin

- Newcomer: Follow the [First Steps with libscid](tutorials/quickstart.md) tutorial to install the wheel and navigate your first chess game.
- Practical Applications: Browse the [How-To Guides](how-to/index.md) to inspect code recipes for parsing PGN, editing variations, or querying game archives.
- Architecture and Design: Read [C ABI Bridge Architecture](explanation/bridge-architecture.md) and [Immutable Cursor Mental Model](explanation/cursor-immutability.md) to understand ownership invariants and navigation semantics.
- API Exploration: Jump directly into the [API Reference](reference/index.md) for detailed symbol signatures and parameter documentation.

---

## 3. Key Architectural Tenets

### 3.1. Immutable Cursor Navigation
Navigation operations across the move tree do not mutate cursors in place. Calling [`Cursor.next()`][libscid.Cursor.next], [`Cursor.previous()`][libscid.Cursor.previous], [`Cursor.enter_variation()`][libscid.Cursor.enter_variation], or [`Cursor.to_game_start()`][libscid.Cursor.to_game_start] yields new [`Cursor`][libscid.Cursor] instances pointing to the requested positions. Tree mutations ([`append_move`][libscid.Cursor.append_move], [`add_variation`][libscid.Cursor.add_variation], [`set_comment`][libscid.Cursor.set_comment]) mutate the underlying game tree predictably.

### 3.2. Event-Driven Movetext Traversal
The [`iter_movetext()`][libscid.iter_movetext] generator yields a structured sequence of [`MovetextEvent`][libscid.MovetextEvent] instances ([`MovetextLineStart`][libscid.MovetextLineStart], [`MovetextMove`][libscid.MovetextMove], [`MovetextLineEnd`][libscid.MovetextLineEnd]), making it straightforward to render commentary, format PGN, or analyse branching variation structures without manual recursion.

### 3.3. FIDE Tournament Arbiter
The [`Arbiter`][libscid.Arbiter] component provides evaluation for standard tournament draw claims at any point along a game line:
- Fifty-Move Rule ([`can_claim_fifty_move_rule`][libscid.Arbiter.can_claim_fifty_move_rule]): 50 full moves (100 plies) elapsed without pawn advance or piece capture.
- Threefold Repetition ([`can_claim_threefold_repetition`][libscid.Arbiter.can_claim_threefold_repetition]): The exact board position (piece placement, active side, castling rights, and en passant target) has occurred at least three times along the current line of play.

### 3.4. Fast Database Indexing and Multi-Criteria Search
`libscid` indexes PGN archives into memory, allowing instant header lookups without parsing full movetext. Queries can be executed with [`HeaderCriteria`][libscid.HeaderCriteria] across dynamic [`Filter`][libscid.Filter] subsets with sorted row mapping.

---

## 4. Quick Start Preview

```python
import libscid

# Parse a game from PGN text:
game = libscid.Game.from_pgn(
    '[Event "Immortal Game"]\n'
    '[White "Adolf Anderssen"]\n'
    '[Black "Lionel Kieseritzky"]\n'
    '[Result "1-0"]\n\n'
    "1. e4 e5 2. f4 exf4 3. Bc4 Qh4+ 4. Kf1 b5 5. Bxb5 Nf6 6. Nf3 Qh6 1-0"
)

# Traverse moves sequentially using an immutable cursor:
cursor = game.create_cursor()
while not cursor.is_line_end:
    cursor = cursor.next()
    print(f"Move: {cursor.previous_move_san:<7} FEN: {cursor.position.fen}")
```
