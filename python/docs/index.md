# libscid Python Documentation

`libscid` is a high-performance chess library providing modern Python bindings built atop the standalone `libscid` C ABI. It delivers fast game representation, standard-compliant PGN parsing and serialization, functional navigation cursors, board analysis, tournament arbiter adjudication, and multi-criteria database indexing and search.

---

## 1. Key Architectural Tenets

### 1.1. Immutable Cursor Navigation
Navigation operations across the move tree do not mutate cursors in place. Calling [`Cursor.next()`][libscid.Cursor.next], [`Cursor.previous()`][libscid.Cursor.previous], [`Cursor.enter_variation()`][libscid.Cursor.enter_variation], or [`Cursor.to_game_start()`][libscid.Cursor.to_game_start] yields new [`Cursor`][libscid.Cursor] instances pointing to the requested positions. Tree mutations ([`append_move`][libscid.Cursor.append_move], [`add_variation`][libscid.Cursor.add_variation], [`set_comment`][libscid.Cursor.set_comment]) mutate the underlying game tree predictably.

### 1.2. Event-Driven Movetext Traversal
The [`iter_movetext()`][libscid.iter_movetext] generator yields a structured sequence of [`MovetextEvent`][libscid.MovetextEvent] instances ([`MovetextLineStart`][libscid.MovetextLineStart], [`MovetextMove`][libscid.MovetextMove], [`MovetextLineEnd`][libscid.MovetextLineEnd]), making it straightforward to render commentary, format PGN, or analyse branching variation structures without manual recursion.

### 1.3. FIDE Tournament Arbiter
The [`Arbiter`][libscid.Arbiter] component provides evaluation for standard tournament draw claims at any point along a game line:
- **Fifty-Move Rule** ([`can_claim_fifty_move_rule`][libscid.Arbiter.can_claim_fifty_move_rule]): 50 full moves (100 ply) elapsed without pawn advance or piece capture.
- **Threefold Repetition** ([`can_claim_threefold_repetition`][libscid.Arbiter.can_claim_threefold_repetition]): The exact board position (piece placement, active side, castling rights, and en passant target) has occurred $\ge 3$ times along the current line of play.

### 1.4. Fast Database Indexing & Multi-Criteria Search
`libscid` indexes PGN archives and Scid binary databases into memory, allowing instant header lookups without parsing full movetext. Queries can be executed with [`HeaderCriteria`][libscid.HeaderCriteria] across dynamic [`Filter`][libscid.Filter] subsets with sorted row mapping.

---

## 2. Quick Start & Practical Examples

### 2.1. Parsing & Navigating PGN Movetext

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

### 2.2. Stream Iteration Over Tree Variations

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 (1... c5 2. Nf3) 2. Nf3 Nc6 *")

# Iterate over all movetext events:
for event in game.iter_movetext():
    match event:
        case libscid.MovetextLineStart(variation_depth=depth):
            indent = "  " * depth
            print(f"{indent}[Start Variation - Depth {depth}]")
        case libscid.MovetextMove(san=san, nags=nags, comment=comment):
            print(f"{indent}Move: {san} (NAGs: {nags}, Comment: {comment})")
        case libscid.MovetextLineEnd():
            print(f"{indent}[End Variation]")
```

### 2.3. Querying Databases and Applying Filters

```python
import libscid

# Open a PGN archive in read-only mode:
database = libscid.Database.open_pgn_read_only("championships.pgn")
print(f"Total indexed games: {database.game_count}")

# Define multi-criteria search parameters:
criteria = libscid.HeaderCriteria(
    white="Kasparov",
    result="1-0",
    eco_min="B80",
    eco_max="B89",
)

# Execute query into a new destination filter:
results = database.search.headers(criteria)
print(f"Matching games found: {results.game_count}")

# Fetch top 10 game indices sorted by date:
indices = results.get_game_indices(sort_criteria="D-", start_row=0, row_count=10)
for index in indices:
    white = database.get_tag(index, "White")
    black = database.get_tag(index, "Black")
    date = database.get_tag(index, "Date")
    print(f"[{date}] {white} vs {black}")
```

---

## 3. Documentation & Reference

- [API Reference](reference.md): Detailed API documentation for all public classes, methods, and types.

