import libscid
import pytest


STANDARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


def test_game_can_create_cursor():
    game = libscid.Game.from_pgn("1. e4 e5 *")

    assert isinstance(game.create_cursor(), libscid.Cursor)


def test_cursor_can_clone():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert isinstance(cursor.clone(), libscid.Cursor)


def test_cursor_exposes_ply():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.ply == 0


def test_cursor_exposes_variation_count():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()

    assert cursor.variation_count == 1


def test_cursor_exposes_variation_depth():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.variation_depth == 0


def test_cursor_exposes_variation_index():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.variation_index == 0


def test_cursor_reports_main_line():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.is_main_line is True


def test_cursor_reports_not_variation_line():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.is_variation_line is False


def test_cursor_reports_line_start():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.is_line_start is True


def test_cursor_reports_not_line_end():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.is_line_end is False


def test_cursor_exposes_position():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.position.fen == STANDARD_FEN


def test_cursor_is_returned_by_libscid_apis():
    with pytest.raises(TypeError, match="Cursor objects are returned by libscid APIs"):
        libscid.Cursor()
