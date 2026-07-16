import libscid
import pytest


STANDARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


def test_game_exposes_start_position():
    game = libscid.Game.from_pgn("1. e4 e5 *")

    assert game.start_position.fen == STANDARD_FEN


def test_game_exposes_end_position():
    game = libscid.Game.from_pgn("1. e4 e5 *")

    assert game.end_position.fen == (
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"
    )


def test_position_can_be_created_from_fen():
    position = libscid.Position.from_fen(STANDARD_FEN)

    assert position.fen == STANDARD_FEN


def test_position_from_fen_rejects_invalid_fen():
    with pytest.raises(libscid.LibScidError) as raised:
        libscid.Position.from_fen("not-a-fen")

    assert raised.value.function == "scid_position_create_from_fen"


def test_position_is_returned_by_libscid_apis():
    with pytest.raises(TypeError, match="Position objects are returned by libscid APIs"):
        libscid.Position()
