# How to Stream Movetext Events

This guide demonstrates how to process game movetext trees as linear event token streams using [`iter_movetext()`][libscid.iter_movetext] and Python structural pattern matching.

---

## 1. The Movetext Event Model

Traversing chess move trees recursively requires maintaining stack state, indent levels, and line pointers. `libscid` simplifies this via an event-driven generator that yields flattened AST tokens derived from [`MovetextEvent`][libscid.MovetextEvent]:

- [`MovetextLineStart`][libscid.MovetextLineStart]: Emitted when entering a mainline or nested variation line. Exposes `variation_depth`.
- [`MovetextMove`][libscid.MovetextMove]: Emitted for each move. Exposes `san`, `move_number`, `is_white`, `nags`, and `comment`.
- [`MovetextLineEnd`][libscid.MovetextLineEnd]: Emitted when reaching the termination of a variation or mainline.

---

## 2. Processing Streams with Structural Pattern Matching

Using Python 3.10+ `match` statements, you can handle event tokens cleanly:

```python
import libscid

pgn = "1. e4 e5 (1... c5 2. Nf3) 2. Nf3 Nc6 3. Bb5 a6 4. Ba4 Nf6 0-1"
game = libscid.Game.from_pgn(pgn)

for event in game.iter_movetext():
    match event:
        case libscid.MovetextLineStart(variation_depth=depth):
            indent = "  " * depth
            if depth > 0:
                print(f"{indent}[Start Variation (Depth {depth})]")

        case libscid.MovetextMove(san=san, move_number=num, is_white=white, nags=nags, comment=cmt):
            color_str = f"{num}." if white else f"{num}..."
            nag_str = f" {nags}" if nags else ""
            cmt_str = f" {{{cmt}}}" if cmt else ""
            print(f"{indent}{color_str} {san}{nag_str}{cmt_str}")

        case libscid.MovetextLineEnd():
            print(f"{indent}[End Line]")
```

Output:

```
1. e4
1... e5
  [Start Variation (Depth 1)]
  1... c5
  2. Nf3
  [End Line]
2. Nf3
2... Nc6
3. Bb5
3... a6
4. Ba4
4... Nf6
[End Line]
```

---

## 3. Customising the Traversal Stream

The generator accepts [`PgnOptions`][libscid.PgnOptions] to include or exclude specific tokens:

```python
import libscid

game = libscid.Game.from_pgn(
    "1. e4 {Best by test.} e5 (1... c5 $1 2. Nf3) 2. Nf3 *"
)

# Stream without comments or variations:
opts = libscid.PgnOptions(
    include_comments=False,
    include_nags=True,
    include_variations=False,
)

for event in game.iter_movetext(options=opts):
    if isinstance(event, libscid.MovetextMove):
        print(f"Move: {event.san}")
```

---

## 4. Extracting Moves into Flat Lists

To extract a clean list of played SAN moves without variation branching:

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 (1... c5) 2. Nf3 Nc6 *")

# Filter only mainline moves:
mainline_moves = [
    event.san
    for event in game.iter_movetext(
        options=libscid.PgnOptions(include_variations=False)
    )
    if isinstance(event, libscid.MovetextMove)
]

print(mainline_moves)  # ['e4', 'e5', 'Nf3', 'Nc6']
```

---

## 5. Related Topics

- For manual cursor navigation, see [How to Navigate and Edit Move Trees](navigate-and-edit-trees.md).
- To understand the architectural trade-offs between generators and tree walkers, read [Explanation: Event Streams vs Recursive Traversal](../explanation/event-stream-vs-tree-walk.md).
- For token specifications, inspect the [PGN and Events Reference](../reference/pgn.md).
