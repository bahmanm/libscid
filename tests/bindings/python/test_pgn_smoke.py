import libscid
import pytest


def _parse_invalid_pgn_error() -> libscid.LibScidError:
    with pytest.raises(libscid.LibScidError) as raised:
        libscid.Game.from_pgn("1. e5 *")
    return raised.value


def test_parse_simple_pgn_exposes_mainline_move_count():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")

    assert game.mainline_move_count == 4


def test_parse_simple_pgn_can_export_movetext():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")

    pgn = game.to_pgn()

    assert "1.e4 e5 2.Nf3 Nc6" in pgn


def test_parse_simple_pgn_can_export_result_marker():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")

    pgn = game.to_pgn()

    assert pgn.rstrip().endswith("*")


def test_parse_invalid_pgn_raises_scid_create_error():
    error = _parse_invalid_pgn_error()

    assert error.function == "scid_game_create"
    assert error.code == 152


def test_parse_invalid_pgn_error_includes_parse_diagnostic():
    error = _parse_invalid_pgn_error()

    assert error.diagnostic is not None
    assert "Failed to parse the move: e5" in error.diagnostic
