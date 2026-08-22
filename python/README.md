<p align="center">
  <img src="https://raw.githubusercontent.com/bahmanm/libscid/main/docs/assets/img/libscid-16x9.png" alt="libscid Hero Banner" width="100%"/>
</p>

# libscid

[![PyPI version](https://img.shields.io/pypi/v/libscid.svg)](https://pypi.org/project/libscid/)
[![Python Versions](https://img.shields.io/pypi/pyversions/libscid.svg)](https://pypi.org/project/libscid/)
[![Documentation](https://img.shields.io/badge/docs-libscid.bahmanm.com%2Fpython-blue)](https://libscid.bahmanm.com/python/)
[![Licence: GPL-2.0](https://img.shields.io/badge/licence-GPL--2.0-green.svg)](https://github.com/bahmanm/libscid/blob/main/COPYING)
[![Typing: Typed](https://img.shields.io/badge/typing-typed-blue.svg)](https://peps.python.org/pep-0561/)

High-performance Python bindings for **libscid**, the standalone C++ chess game representation, PGN manipulation, and database library derived from Scid and ScidUp.

`libscid` provides modern, type-safe, and ergonomic Python abstractions for chess game representation, standard-compliant PGN parsing and serialization, functional move tree navigation, FIDE tournament rule adjudication, position analysis, and multi-criteria database search.

---

## 1. Key Features

- **Immutable Cursor Navigation**: Functional, side-effect-free move tree traversal with explicit mutation semantics for variations and comments.
- **Event-Driven Movetext Streaming**: Hierarchical generator stream (`iter_movetext`) yielding structured events (`MovetextLineStart`, `MovetextMove`, `MovetextLineEnd`) for clean formatting and analysis.
- **FIDE Tournament Arbiter**: Accurate adjudication of standard draw claims along any move sequence, including the **Fifty-Move Rule** and **Threefold Repetition**.
- **High-Performance Database Engine**: In-memory and persistent indexing for Scid databases and PGN archives, enabling rapid header searching, criteria filtering, and sorted index views without parsing full movetext.
- **Type-Safe & Pythonic**: Fully typed (PEP 561 compliant), modern Python API (>= 3.10) designed around the principle of Least Surprise.

---

## 2. Installation

Install the pre-compiled binary wheel from PyPI:

```sh
pip install libscid
```

---

## 3. Quick Start

### 3.1. Parsing PGN and Navigating Moves

```python
import libscid

# Parse a game from PGN string
pgn_data = """[Event "World Championship"]
[Site "Reykjavik ISL"]
[Date "1972.07.23"]
[Round "6"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1-0"]

1. c4 e6 2. Nf3 d5 3. d4 Nf6 4. Nc3 Be7 5. Bg5 O-O 6. e3 h6 7. Bh4 b6 1-0
"""

game = libscid.Game.from_pgn(pgn_data)
print(f"{game.get_tag('White')} vs {game.get_tag('Black')} ({game.get_tag('Result')})")

# Traverse moves sequentially using an immutable cursor
cursor = game.create_cursor()
while not cursor.is_line_end:
    cursor = cursor.next()
    print(f"Move: {cursor.previous_move_san:<6} FEN: {cursor.position.fen}")
```

### 3.2. Hierarchical Movetext & Variation Streaming

```python
import libscid

game = libscid.Game.from_pgn("1. e4 e5 (1... c5 2. Nf3) 2. Nf3 Nc6 *")

for event in game.iter_movetext():
    match event:
        case libscid.MovetextLineStart(variation_depth=depth):
            indent = "  " * depth
            print(f"{indent}[Start Variation (depth {depth})]")
        case libscid.MovetextMove(san=san, comment=comment):
            print(f"{indent}Move: {san}" + (f" ({comment})" if comment else ""))
        case libscid.MovetextLineEnd():
            print(f"{indent}[End Variation]")
```

### 3.3. FIDE Tournament Rule Adjudication

```python
import libscid

# Position repeated 3 times in standard play
pgn = "1. Nf3 Nf6 2. Ng1 Ng8 3. Nf3 Nf6 4. Ng1 Ng8 *"
cursor = libscid.Game.from_pgn(pgn).create_cursor().to_game_end()

print("Threefold repetition:", cursor.arbiter.can_claim_threefold_repetition)  # True
print("Fifty-move rule:", cursor.arbiter.can_claim_fifty_move_rule)            # False
```

### 3.4. Database Indexing and Multi-Criteria Filtering

```python
import libscid

# Open a PGN archive in read-only mode with instant header indexing
db = libscid.Database.open_pgn_read_only("games.pgn")
print(f"Total indexed games: {db.game_count}")

# Query using structured criteria
criteria = libscid.HeaderCriteria(
    white="Kasparov",
    result="1-0",
    eco_min="B80",
    eco_max="B89",
)

results = db.search.headers(criteria)
print(f"Matching games: {results.game_count}")

# Fetch top results sorted by date descending
for index in results.get_game_indices(sort_criteria="D-", start_row=0, row_count=5):
    print(f"{db.get_tag(index, 'Date')}: {db.get_tag(index, 'White')} vs {db.get_tag(index, 'Black')}")
```

---

## 4. Documentation & Resources

- **Full Documentation & Guides**: [https://libscid.bahmanm.com/python/](https://libscid.bahmanm.com/python/)
- **API Reference**: [https://libscid.bahmanm.com/python/reference/](https://libscid.bahmanm.com/python/reference/)
- **C ABI Documentation**: [https://libscid.bahmanm.com/capi/](https://libscid.bahmanm.com/capi/)
- **Source Code & Issue Tracker**: [https://github.com/bahmanm/libscid](https://github.com/bahmanm/libscid)

---

## 5. Licence

`libscid` is released under the GNU General Public License v2.0 (GPL-2.0-only). See the [COPYING](https://github.com/bahmanm/libscid/blob/main/COPYING) file for full licence details.
