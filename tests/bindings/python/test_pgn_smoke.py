import libscid
import pytest


def test_parse_simple_pgn_exposes_mainline_halfmove_count():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")

    assert game.mainline_halfmove_count == 4


def test_parse_simple_pgn_can_export_text():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")

    pgn = game.to_pgn()

    assert "1.e4 e5 2.Nf3 Nc6" in pgn
    assert pgn.rstrip().endswith("*")


def test_parse_invalid_pgn_raises_with_diagnostic():
    with pytest.raises(libscid.LibScidError) as raised:
        libscid.Game.from_pgn("1. e5 *")

    assert raised.value.function == "scid_game_create"
    assert raised.value.code == 152
    assert raised.value.diagnostic is not None
    assert "Failed to parse the move: e5" in raised.value.diagnostic
    assert "End of game, ignored the part after the last error." in raised.value.diagnostic
