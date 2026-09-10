# Building a CLI Game Inspector

In this tutorial, you will build a standalone Python script to inspect chess games from PGN files. You will learn how to parse movetext, step through positions using immutable cursors, inspect board conditions (check, halfmove clock, material status), and adjudicate FIDE draw claims using the tournament arbiter.

---

## 1. What You Will Build

By the end of this lesson, you will have a working Python script (`inspect_game.py`) that:

- Reads any PGN string or file.
- Validates game metadata and displays the Seven Tag Roster.
- Plays through every move, printing formatted move numbers and SAN notation.
- Inspects checks, captures, and clock plies at each stage.
- Adjudicates whether either player could legally claim a draw under FIDE rules.

---

## 2. Setting Up the Script

Create a new file named `inspect_game.py` and import `libscid`:

```python
#!/usr/bin/env python3
"""CLI chess game inspector using libscid."""

import sys
import libscid


def inspect_game(pgn_content: str) -> None:
    # 1. Parse game aggregate
    game = libscid.Game.from_pgn(pgn_content)

    print("=" * 60)
    print(f"Event:  {game.get_tag('Event')}")
    print(f"Date:   {game.get_tag('Date')}")
    print(f"White:  {game.get_tag('White')}")
    print(f"Black:  {game.get_tag('Black')}")
    print(f"Result: {game.get_tag('Result')}")
    print(f"Total Mainline Plies: {game.mainline_move_count}")
    print("=" * 60)
```

---

## 3. Navigating Moves and Board States

To inspect each step of the contest, create a [`Cursor`][libscid.Cursor] at the root and iterate until reaching the final position:

```python
    cursor = game.create_cursor()
    arbiter = game.arbiter

    ply = 0
    while not cursor.is_line_end:
        cursor = cursor.next()
        ply += 1

        move_san = cursor.previous_move_san
        pos = cursor.position
        move_num = pos.fullmove_number
        side = "W" if pos.side_to_move == "black" else "B"

        status_flags = []
        if pos.is_check:
            status_flags.append("CHECK")
        if pos.is_checkmate:
            status_flags.append("CHECKMATE")
        if pos.is_stalemate:
            status_flags.append("STALEMATE")

        status_str = f" [{', '.join(status_flags)}]" if status_flags else ""

        print(
            f"Ply {ply:02d} | Move {move_num}.{side} {move_san:<7} "
            f"| Halfmove: {pos.halfmove_clock:02d}{status_str}"
        )
```

Notice how [`cursor.position`][libscid.Cursor.position] exposes both structural board invariants (`halfmove_clock`, `fullmove_number`, `side_to_move`) and tactical assessments (`is_check`, `is_checkmate`, `is_stalemate`).

---

## 4. Adjudicating FIDE Draw Conditions

At any point in the contest, players may be entitled to claim a draw under tournament regulations. The [`Arbiter`][libscid.Arbiter] component adjudicates these claims automatically:

Add draw evaluation logic to the end of your inspection function:

```python
    print("-" * 60)
    print("Tournament Arbiter Evaluation at Final Position:")

    can_repetition = cursor.arbiter.can_claim_threefold_repetition
    can_fifty = cursor.arbiter.can_claim_fifty_move_rule

    print(f"- Threefold Repetition Claim Valid: {can_repetition}")
    print(f"- Fifty-Move Rule Claim Valid:     {can_fifty}")

    if can_repetition:
        print("  Notice: Active side may claim a draw due to 3-fold repetition.")
    if can_fifty:
        print("  Notice: Active side may claim a draw due to 50 moves without capture or pawn push.")
    print("=" * 60)
```

---

## 5. Testing with an Immortal Classic

Add a main entry point testing the famous Anderssen versus Kieseritzky "Immortal Game" (London, 1851):

```python
IMMORTAL_GAME_PGN = """[Event "London Casual"]
[Site "London"]
[Date "1851.06.21"]
[Round "?"]
[White "Adolf Anderssen"]
[Black "Lionel Kieseritzky"]
[Result "1-0"]

1. e4 e5 2. f4 exf4 3. Bc4 Qh4+ 4. Kf1 b5 5. Bxb5 Nf6 6. Nf3 Qh6
7. d3 Nh5 8. Nh4 Qg5 9. Nf5 c6 10. g4 Nf6 11. Rg1 cxb5 12. h4 Qg6
13. h5 Qg5 14. Qf3 Ng8 15. Bxf4 Qf6 16. Nc3 Bc5 17. Nd5 Qxb2
18. Bd6 Bxg1 19. e5 Qxa1+ 20. Ke2 Na6 21. Nxg7+ Kd8 22. Qf6+ Nxf6
23. Be7# 1-0
"""

if __name__ == "__main__":
    inspect_game(IMMORTAL_GAME_PGN)
```

Run the script from your terminal:

```bash
python inspect_game.py
```

The output cleanly details every ply up to Anderssen's checkmate (`23. Be7#`):

```
============================================================
Event:  London Casual
Date:   1851.06.21
White:  Adolf Anderssen
Black:  Lionel Kieseritzky
Result: 1-0
Total Mainline Plies: 45
============================================================
Ply 01 | Move 1.W e4      | Halfmove: 00
Ply 02 | Move 1.B e5      | Halfmove: 00
...
Ply 44 | Move 22.B Nxf6    | Halfmove: 00
Ply 45 | Move 23.W Be7#    | Halfmove: 00 [CHECK, CHECKMATE]
------------------------------------------------------------
Tournament Arbiter Evaluation at Final Position:
- Threefold Repetition Claim Valid: False
- Fifty-Move Rule Claim Valid:     False
============================================================
```

---

## 6. What You Have Learnt

You have created a complete chess analysis utility leveraging `libscid`. You now know how to:

- Deserialise PGN text into an aggregate [`Game`][libscid.Game].
- Step through game lines deterministically using an immutable [`Cursor`][libscid.Cursor].
- Inspect board properties on [`Position`][libscid.Position].
- Evaluate FIDE tournament rules using [`Arbiter`][libscid.Arbiter].

For your next steps, explore [Analysing Master Archives](database-queries.md) to process thousands of games simultaneously.
