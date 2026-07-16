import libscid
import pytest


STANDARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


def test_game_can_create_cursor():
    game = libscid.Game.from_pgn("1. e4 e5 *")

    assert isinstance(game.create_cursor(), libscid.Cursor)


def test_cursor_can_clone():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert isinstance(cursor.clone(), libscid.Cursor)


def test_cursor_next_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert isinstance(cursor.next(), libscid.Cursor)


def test_cursor_next_advances_to_next_move():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.next().previous_move_san == "e4"


def test_cursor_next_does_not_mutate_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    cursor.next()

    assert cursor.previous_move_san is None


def test_cursor_next_returns_none_at_line_end():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()
    cursor = cursor.next()
    cursor = cursor.next()

    assert cursor.next() is None


def test_cursor_previous_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert isinstance(cursor.previous(), libscid.Cursor)


def test_cursor_previous_returns_cursor_before_previous_move():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert cursor.previous().next_move_san == "e4"


def test_cursor_previous_does_not_mutate_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    cursor.previous()

    assert cursor.previous_move_san == "e4"


def test_cursor_previous_returns_none_at_line_start():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.previous() is None


def test_cursor_enter_variation_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()

    assert isinstance(cursor.enter_variation(0), libscid.Cursor)


def test_cursor_enter_variation_enters_variation_line():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()

    assert cursor.enter_variation(0).is_variation_line is True


def test_cursor_enter_variation_does_not_mutate_cursor():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()

    cursor.enter_variation(0)

    assert cursor.is_main_line is True


def test_cursor_enter_variation_returns_none_for_missing_variation():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.enter_variation(0) is None


def test_cursor_exit_variation_returns_parent_cursor():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    assert variation.exit_variation().is_main_line is True


def test_cursor_exit_variation_does_not_mutate_cursor():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    variation.exit_variation()

    assert variation.is_variation_line is True


def test_cursor_exit_variation_returns_none_on_main_line():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.exit_variation() is None


def test_cursor_exposes_next_move_san():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.next_move_san == "e4"


def test_cursor_exposes_previous_move_san():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert cursor.previous_move_san == "e4"


def test_cursor_next_move_san_returns_none_at_line_end():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()
    cursor = cursor.next()
    cursor = cursor.next()

    assert cursor.next_move_san is None


def test_cursor_previous_move_san_returns_none_at_line_start():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.previous_move_san is None


def test_cursor_next_move_san_reads_variation_line():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()

    assert cursor.enter_variation(0).next_move_san == "d4"


def test_cursor_exposes_next_move_nags():
    cursor = libscid.Game.from_pgn("1. e4 $1 e5 *").create_cursor()

    assert cursor.next_move_nags == (1,)


def test_cursor_exposes_previous_move_nags():
    cursor = libscid.Game.from_pgn("1. e4 $1 e5 *").create_cursor().next()

    assert cursor.previous_move_nags == (1,)


def test_cursor_move_nags_preserve_order():
    cursor = libscid.Game.from_pgn("1. e4 $1 $146 e5 *").create_cursor()

    assert cursor.next_move_nags == (1, 146)


def test_cursor_next_move_nags_returns_none_at_line_end():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()
    cursor = cursor.next()
    cursor = cursor.next()

    assert cursor.next_move_nags is None


def test_cursor_previous_move_nags_returns_none_at_line_start():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.previous_move_nags is None


def test_cursor_next_move_nags_preserve_empty_nags():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.next_move_nags == ()


def test_cursor_preceding_comment_reads_initial_comment():
    cursor = libscid.Game.from_pgn("{Before game} 1. e4 e5 *").create_cursor()

    assert cursor.preceding_comment == "Before game"


def test_cursor_preceding_comment_reads_variation_initial_comment():
    cursor = libscid.Game.from_pgn(
        "1. e4 ({Branch} 1. d4 d5) e5 *"
    ).create_cursor()

    assert cursor.enter_variation(0).preceding_comment == "Branch"


def test_cursor_preceding_comment_returns_none_after_line_start():
    cursor = libscid.Game.from_pgn("1. e4 {King pawn} e5 *").create_cursor().next()

    assert cursor.preceding_comment is None


def test_cursor_preceding_comment_preserves_empty_comment():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.preceding_comment == ""


def test_cursor_comment_reads_previous_move_comment():
    cursor = libscid.Game.from_pgn("1. e4 {King pawn} e5 *").create_cursor().next()

    assert cursor.comment == "King pawn"


def test_cursor_comment_returns_none_at_line_start():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.comment is None


def test_cursor_comment_preserves_empty_comment():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert cursor.comment == ""


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
