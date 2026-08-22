# Movetext Event Iteration

`libscid` provides an event-driven stream generator, [`iter_movetext()`][libscid.iter_movetext], to traverse complex PGN movetext and variation trees without manual recursion.

---

## 1. Event Model

The stream emits four event types derived from [`MovetextEvent`][libscid.MovetextEvent]:

1. **[`MovetextLineStart`][libscid.MovetextLineStart]**: Emitted when entering a line (mainline or nested variation) with the current `variation_depth`.
2. **[`MovetextMove`][libscid.MovetextMove]**: Emitted for each move along the line, containing SAN notation, move number, active color, NAG annotations, and comments.
3. **[`MovetextLineEnd`][libscid.MovetextLineEnd]**: Emitted when reaching the end of the current variation or mainline.
4. **[`MovetextEvent`][libscid.MovetextEvent]**: Base class for all movetext event tokens.

---

## 2. Stream Processing with Pattern Matching

Using Python 3.10+ structural pattern matching, processing game trees is clean and declarative:

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
            print(f"{indent}[End of Line]")
```

---

## 3. Options for Event Stream

The generator accepts [`PgnOptions`][libscid.PgnOptions] to customise formatting and traversal:

```python
import libscid

game = libscid.Game.from_pgn("1. e4 {Best by test.} e5 *")

# Configure PGN generation options
opts = libscid.PgnOptions(
    include_comments=True,
    include_nags=True,
    include_variations=True,
)

for event in game.iter_movetext(options=opts):
    if isinstance(event, libscid.MovetextMove):
        print(f"Move SAN: {event.san}")
```
