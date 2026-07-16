import libscid


def test_parse_simple_pgn_exposes_mainline_halfmove_count():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")

    assert game.mainline_halfmove_count == 4


def test_parse_simple_pgn_can_export_text():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")

    pgn = game.to_pgn()

    assert "1.e4 e5 2.Nf3 Nc6" in pgn
    assert pgn.rstrip().endswith("*")
