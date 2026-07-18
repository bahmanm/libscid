import pytest

import libscid

ANNOTATED_PGN = """[Event "Options"]
[Site "Internet"]
[Date "2026.07.16"]
[Round "1"]
[White "White"]
[Black "Black"]
[Result "*"]
[Annotator "Python"]
[ECO "C20"]

{Before start} 1. e4 $1 {King pawn} (1. c4 {English}) e5 {Reply} *
"""


def test_default_pgn_options_match_default_export():
    game = libscid.Game.from_pgn(ANNOTATED_PGN)

    assert game.to_pgn(libscid.PgnOptions()) == game.to_pgn()


def test_pgn_options_can_export_symbolic_nags():
    game = libscid.Game.from_pgn(ANNOTATED_PGN)

    pgn = game.to_pgn(libscid.PgnOptions(symbolic_nags=True))

    assert "1.e4 ! {King pawn}" in pgn


def test_pgn_options_can_exclude_supplemental_tags():
    game = libscid.Game.from_pgn(ANNOTATED_PGN)

    pgn = game.to_pgn(libscid.PgnOptions(supplemental_tags=False))

    assert '[Annotator "Python"]' not in pgn
    assert '[ECO "C20"]' not in pgn


def test_pgn_options_can_exclude_comments():
    game = libscid.Game.from_pgn(ANNOTATED_PGN)

    pgn = game.to_pgn(libscid.PgnOptions(comments=False))

    assert "{Before start}" not in pgn
    assert "{King pawn}" not in pgn


def test_pgn_options_can_exclude_nags():
    game = libscid.Game.from_pgn(ANNOTATED_PGN)

    pgn = game.to_pgn(libscid.PgnOptions(comments=False))

    assert "$1" not in pgn


def test_pgn_options_can_exclude_variations():
    game = libscid.Game.from_pgn(ANNOTATED_PGN)

    pgn = game.to_pgn(libscid.PgnOptions(variations=False))

    assert "(1.c4" not in pgn
    assert "{English}" not in pgn


def test_pgn_options_can_set_line_width():
    game = libscid.Game.from_pgn("1. e4 e5 *")

    pgn = game.to_pgn(libscid.PgnOptions(line_width=5))

    assert "1.e4\ne5" in pgn


def test_pgn_options_reject_negative_line_width():
    with pytest.raises(ValueError, match="line_width must be non-negative or None"):
        libscid.PgnOptions(line_width=-1)
