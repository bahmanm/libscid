# How to Parse and Serialise PGN

This guide demonstrates how to parse PGN text, manage Seven Tag Roster (STR) headers and supplemental tags, configure PGN export options, and serialise games.

---

## 1. Parsing PGN Text

Use [`Game.from_pgn`][libscid.Game.from_pgn] to parse a complete PGN string:

```python
import libscid

pgn_data = """[Event "Candidates Tournament"]
[Site "Toronto CAN"]
[Date "2024.04.14"]
[Round "9"]
[White "Vidit, Santosh Gujrathi"]
[Black "Nakamura, Hikaru"]
[Result "1-0"]
[ECO "C55"]

1. e4 e5 2. Nf3 Nc6 3. Bc4 Nf6 4. d3 Be7 5. O-O O-O 6. Re1 d6 1-0
"""

game = libscid.Game.from_pgn(pgn_data)
print(f"White: {game.get_tag('White')}")
print(f"Black: {game.get_tag('Black')}")
print(f"Mainline plies: {game.mainline_move_count}")
```

---

## 2. Reading and Writing PGN Tags

Manage standard and supplemental PGN header tags using [`get_tag`][libscid.Game.get_tag], [`set_tag`][libscid.Game.set_tag], and [`remove_tag`][libscid.Game.remove_tag]:

```python
# Inspect tags:
white = game.get_tag("White")  # 'Vidit, Santosh Gujrathi'
annotator = game.get_tag("Annotator")  # None if tag does not exist

# Attach or modify tags:
game.set_tag("Annotator", "GM Analysis")
game.set_tag("WhiteElo", "2727")
game.set_tag("BlackElo", "2789")

# Remove a tag:
game.remove_tag("Annotator")
```

---

## 3. Initialising Games with Non-Standard Starting Positions

To create a game starting from an arbitrary board setup (such as a chess problem or endgame study), initialise [`Game`][libscid.Game] with a custom [`Position`][libscid.Position]:

```python
import libscid

# Custom endgame position:
custom_fen = "8/8/8/4k3/8/8/4K3/4R3 w - - 0 1"
position = libscid.Position(custom_fen)

game = libscid.Game(position=position)
print(f"FEN tag present: {game.get_tag('FEN')}")  # Set automatically
print(f"SetUp tag present: {game.get_tag('SetUp')}")  # '1'
```

---

## 4. Customising PGN Serialisation with PgnOptions

The [`to_pgn`][libscid.Game.to_pgn] method accepts [`PgnOptions`][libscid.PgnOptions] to fine-tune output formatting:

```python
import libscid

game = libscid.Game.from_pgn(
    "1. e4 {Best by test.} e5 (1... c5 $1) 2. Nf3 *"
)

# Export without comments or NAGs:
clean_options = libscid.PgnOptions(
    include_comments=False,
    include_nags=False,
    include_variations=True,
)
print(game.to_pgn(options=clean_options))
# Outputs: 1. e4 e5 (1... c5) 2. Nf3 *

# Export stripped mainline only:
mainline_only = libscid.PgnOptions(
    include_comments=False,
    include_nags=False,
    include_variations=False,
)
print(game.to_pgn(options=mainline_only))
# Outputs: 1. e4 e5 2. Nf3 *
```

---

## 5. Related Topics

- For event-driven movetext streaming, see [How to Stream Movetext Events](stream-movetext-events.md).
- For branching lines and commentary insertion, see [How to Navigate and Edit Move Trees](navigate-and-edit-trees.md).
- For complete API signatures, consult the [PGN and Notation Reference](../reference/pgn.md).
