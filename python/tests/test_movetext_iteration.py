import pytest

import libscid


def event_moves(events):
    return [event.san for event in events if isinstance(event, libscid.MovetextMove)]


def event_types(events):
    return [type(event) for event in events]


def test_game_iter_movetext_emits_mainline_events():
    game = libscid.Game.from_pgn("{Before start} 1. e4 $1 {King pawn} e5 *")

    events = list(game.iter_movetext())

    assert event_types(events) == [
        libscid.MovetextLineStart,
        libscid.MovetextMove,
        libscid.MovetextMove,
        libscid.MovetextLineEnd,
    ]
    assert events[0].preceding_comment == "Before start"
    assert events[0].variation_depth == 0
    assert events[0].variation_index == 0
    assert events[0].cursor.is_line_start is True
    assert event_moves(events) == ["e4", "e5"]


def test_movetext_event_type_is_public():
    assert libscid.MovetextEvent == (
        libscid.MovetextLineStart | libscid.MovetextMove | libscid.MovetextLineEnd
    )


def test_movetext_move_event_exposes_move_metadata_and_positions():
    game = libscid.Game.from_pgn("1. e4 $1 {King pawn} e5 *")

    move = next(
        event
        for event in game.iter_movetext()
        if isinstance(event, libscid.MovetextMove)
    )

    assert move.san == "e4"
    assert move.uci == "e2e4"
    assert move.nags == (libscid.Nag(1),)
    assert move.comment == "King pawn"
    assert move.variation_depth == 0
    assert move.variation_index == 0
    assert move.before.next_move_san == "e4"
    assert move.after.previous_move_san == "e4"
    assert move.position_before.side_to_move == "white"
    assert move.position_after.side_to_move == "black"


def test_game_iter_movetext_enters_variations_after_parent_move():
    game = libscid.Game.from_pgn(
        "1. e4 {King pawn} ({Queen} 1. d4 {Queen pawn} d5) e5 *"
    )

    events = list(game.iter_movetext())

    assert event_types(events) == [
        libscid.MovetextLineStart,
        libscid.MovetextMove,
        libscid.MovetextLineStart,
        libscid.MovetextMove,
        libscid.MovetextMove,
        libscid.MovetextLineEnd,
        libscid.MovetextMove,
        libscid.MovetextLineEnd,
    ]
    assert event_moves(events) == ["e4", "d4", "d5", "e5"]
    assert events[2].preceding_comment == "Queen"
    assert events[2].variation_depth == 1
    assert events[2].variation_index == 0
    assert events[5].variation_depth == 1
    assert events[7].variation_depth == 0


def test_game_iter_movetext_can_skip_variations():
    game = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *")

    events = list(game.iter_movetext(variations=False))

    assert event_moves(events) == ["e4", "e5"]
    assert event_types(events) == [
        libscid.MovetextLineStart,
        libscid.MovetextMove,
        libscid.MovetextMove,
        libscid.MovetextLineEnd,
    ]


def test_cursor_iter_movetext_starts_from_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *").create_cursor().next()

    events = list(cursor.iter_movetext())

    assert event_types(events) == [
        libscid.MovetextLineStart,
        libscid.MovetextMove,
        libscid.MovetextMove,
        libscid.MovetextLineEnd,
    ]
    assert events[0].preceding_comment is None
    assert events[0].cursor.previous_move_san == "e4"
    assert event_moves(events) == ["e5", "Nf3"]


def test_cursor_iter_movetext_emits_empty_segment_at_line_end():
    cursor = libscid.Game.from_pgn("1. e4 *").create_cursor().to_game_end()

    events = list(cursor.iter_movetext())

    assert event_types(events) == [
        libscid.MovetextLineStart,
        libscid.MovetextLineEnd,
    ]
    assert events[0].cursor.is_line_end is True
    assert events[1].cursor.is_line_end is True


def test_cursor_iter_movetext_rejects_non_bool_variations():
    cursor = libscid.Game.from_pgn("1. e4 *").create_cursor()

    with pytest.raises(TypeError, match="variations must be bool"):
        list(cursor.iter_movetext(variations="yes"))
