import pytest

import libscid

STANDARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
AFTER_E4_FEN = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"


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


def test_position_exposes_white_side_to_move():
    position = libscid.Position.from_fen(STANDARD_FEN)

    assert position.side_to_move == "white"


def test_position_exposes_black_side_to_move():
    position = libscid.Position.from_fen(AFTER_E4_FEN)

    assert position.side_to_move == "black"


def test_position_exposes_fullmove_number():
    position = libscid.Position.from_fen("8/K7/8/8/7k/8/8/8 w - - 45 25")

    assert position.fullmove_number == 25


def test_position_exposes_halfmove_clock():
    position = libscid.Position.from_fen("8/K7/8/8/7k/8/8/8 w - - 45 25")

    assert position.halfmove_clock == 45


def test_position_get_piece_at_returns_white_piece_symbol():
    position = libscid.Position.from_fen(STANDARD_FEN)

    assert position.get_piece_at("e1") == "K"


def test_position_get_piece_at_returns_black_piece_symbol():
    position = libscid.Position.from_fen(STANDARD_FEN)

    assert position.get_piece_at(b"a8") == "r"


def test_position_get_piece_at_returns_none_for_empty_square():
    position = libscid.Position.from_fen(STANDARD_FEN)

    assert position.get_piece_at("e4") is None


def test_position_get_piece_at_reads_mutated_position():
    position = libscid.Position.from_fen(STANDARD_FEN)

    position.apply_san("e4")

    assert position.get_piece_at("e4") == "P"


def test_position_get_piece_at_rejects_invalid_square():
    position = libscid.Position.from_fen(STANDARD_FEN)

    with pytest.raises(libscid.LibScidError) as raised:
        position.get_piece_at("i9")

    assert raised.value.function == "scid_square_from_string"


def test_position_from_fen_rejects_invalid_fen():
    with pytest.raises(libscid.LibScidError) as raised:
        libscid.Position.from_fen("not-a-fen")

    assert raised.value.function == "scid_position_create_from_fen"


def test_position_can_apply_san():
    position = libscid.Position.from_fen(STANDARD_FEN)

    result = position.apply_san("e4")

    assert result is None
    assert position.fen == AFTER_E4_FEN
    assert position.side_to_move == "black"


def test_position_can_apply_uci():
    position = libscid.Position.from_fen(STANDARD_FEN)

    result = position.apply_uci("e2e4")

    assert result is None
    assert position.fen == AFTER_E4_FEN
    assert position.side_to_move == "black"


def test_position_apply_san_rejects_invalid_move_without_mutating():
    position = libscid.Position.from_fen(STANDARD_FEN)

    with pytest.raises(libscid.LibScidError) as raised:
        position.apply_san("e5")

    assert raised.value.function == "scid_position_apply_san"
    assert position.fen == STANDARD_FEN


def test_position_apply_uci_rejects_invalid_move_without_mutating():
    position = libscid.Position.from_fen(STANDARD_FEN)

    with pytest.raises(libscid.LibScidError) as raised:
        position.apply_uci("e7e5")

    assert raised.value.function == "scid_position_apply_uci"
    assert position.fen == STANDARD_FEN


def test_position_is_returned_by_libscid_apis():
    with pytest.raises(
        TypeError, match="Position objects are returned by libscid APIs"
    ):
        libscid.Position()
