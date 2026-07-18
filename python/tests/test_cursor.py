import pytest

import libscid

STANDARD_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


def _game_from_position(position: libscid.Position, movetext: str) -> libscid.Game:
    game = libscid.Game(position=position)
    cursor = game.create_cursor()
    for san in movetext.split():
        cursor = cursor.append_move(san)
    return game


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


def test_cursor_to_game_start_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert isinstance(cursor.to_game_start(), libscid.Cursor)


def test_cursor_to_game_start_returns_start_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert cursor.to_game_start().previous_move_san is None


def test_cursor_to_game_start_does_not_mutate_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    cursor.to_game_start()

    assert cursor.previous_move_san == "e4"


def test_cursor_to_game_end_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert isinstance(cursor.to_game_end(), libscid.Cursor)


def test_cursor_to_game_end_returns_end_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.to_game_end().next_move_san is None


def test_cursor_to_game_end_does_not_mutate_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    cursor.to_game_end()

    assert cursor.next_move_san == "e4"


def test_cursor_to_game_end_uses_main_line_from_variation():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    assert variation.to_game_end().previous_move_san == "e5"


def test_cursor_to_main_line_offset_returns_start_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert cursor.to_main_line_offset(0).previous_move_san is None


def test_cursor_to_main_line_offset_returns_cursor_after_offset_moves():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.to_main_line_offset(1).previous_move_san == "e4"


def test_cursor_to_main_line_offset_can_return_line_end_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.to_main_line_offset(2).next_move_san is None


def test_cursor_to_main_line_offset_does_not_mutate_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    cursor.to_main_line_offset(1)

    assert cursor.previous_move_san is None


def test_cursor_to_main_line_offset_returns_none_for_missing_offset():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.to_main_line_offset(3) is None


def test_cursor_to_main_line_offset_uses_main_line_from_variation():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    assert variation.to_main_line_offset(2).previous_move_san == "e5"


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


def test_cursor_add_variation_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert isinstance(cursor.add_variation(), libscid.Cursor)


def test_cursor_add_variation_enters_variation_line():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.add_variation().is_variation_line is True


def test_cursor_add_variation_sets_preceding_comment():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    variation = cursor.add_variation("Queen pawn")

    assert variation.preceding_comment == "Queen pawn"


def test_cursor_add_variation_updates_variation_count():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    cursor.add_variation()

    assert cursor.variation_count == 1


def test_cursor_add_variation_returns_none_at_line_end():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().to_game_end()

    assert cursor.add_variation() is None


def test_cursor_remove_variation_returns_parent_cursor():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    assert variation.remove_variation().is_main_line is True


def test_cursor_remove_variation_updates_variation_count():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    parent = variation.remove_variation()

    assert parent.variation_count == 0


def test_cursor_remove_variation_removes_current_variation():
    cursor = libscid.Game.from_pgn(
        "1. e4 ({Queen} 1. d4 d5) ({English} 1. c4 c5) e5 *"
    ).create_cursor()
    variation = cursor.enter_variation(1)

    parent = variation.remove_variation()

    assert parent.enter_variation(0).preceding_comment == "Queen"


def test_cursor_remove_variation_returns_none_on_main_line():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.remove_variation() is None


def test_cursor_promote_variation_to_first_returns_new_cursor():
    cursor = libscid.Game.from_pgn(
        "1. e4 ({Queen} 1. d4 d5) ({English} 1. c4 c5) e5 *"
    ).create_cursor()
    variation = cursor.enter_variation(1)

    assert isinstance(variation.promote_variation_to_first(), libscid.Cursor)


def test_cursor_promote_variation_to_first_updates_variation_index():
    cursor = libscid.Game.from_pgn(
        "1. e4 ({Queen} 1. d4 d5) ({English} 1. c4 c5) e5 *"
    ).create_cursor()
    variation = cursor.enter_variation(1)

    promoted = variation.promote_variation_to_first()

    assert promoted.variation_index == 0


def test_cursor_promote_variation_to_first_updates_variation_order():
    cursor = libscid.Game.from_pgn(
        "1. e4 ({Queen} 1. d4 d5) ({English} 1. c4 c5) e5 *"
    ).create_cursor()
    variation = cursor.enter_variation(1)

    parent = variation.promote_variation_to_first().exit_variation()

    assert parent.enter_variation(0).preceding_comment == "English"


def test_cursor_promote_variation_to_first_returns_none_on_main_line():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.promote_variation_to_first() is None


def test_cursor_promote_variation_to_mainline_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 (1. c4 c5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    assert isinstance(variation.promote_variation_to_mainline(), libscid.Cursor)


def test_cursor_promote_variation_to_mainline_returns_main_line_cursor():
    cursor = libscid.Game.from_pgn("1. e4 (1. c4 c5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    promoted = variation.promote_variation_to_mainline()

    assert promoted.is_main_line is True


def test_cursor_promote_variation_to_mainline_uses_variation_as_main_line():
    cursor = libscid.Game.from_pgn("1. e4 (1. c4 c5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    promoted = variation.promote_variation_to_mainline()

    assert promoted.next_move_san == "c4"


def test_cursor_promote_variation_to_mainline_updates_pgn_output():
    game = libscid.Game.from_pgn("1. e4 (1. c4 c5) e5 *")
    variation = game.create_cursor().enter_variation(0)

    variation.promote_variation_to_mainline()

    assert "(1.e4 e5)" in game.to_pgn()


def test_cursor_promote_variation_to_mainline_returns_none_on_main_line():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.promote_variation_to_mainline() is None


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


def test_cursor_append_move_returns_new_cursor():
    cursor = libscid.Game().create_cursor()

    assert isinstance(cursor.append_move("e4"), libscid.Cursor)


def test_cursor_append_move_returns_cursor_after_appended_move():
    cursor = libscid.Game().create_cursor()

    appended = cursor.append_move("e4")

    assert appended.previous_move_san == "e4"


def test_cursor_append_move_updates_game():
    game = libscid.Game()

    game.create_cursor().append_move("e4")

    assert game.mainline_move_count == 1


def test_cursor_append_move_requires_line_end():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    with pytest.raises(ValueError, match="append_move requires cursor at line end"):
        cursor.append_move("c5")


def test_cursor_append_move_rejects_illegal_san():
    cursor = libscid.Game().create_cursor()

    with pytest.raises(libscid.LibScidError) as raised:
        cursor.append_move("e5")

    assert raised.value.function == "scid_movespec_create_from_san"


def test_cursor_truncate_then_append_move_replaces_suffix():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *")
    cursor = game.create_cursor().next()

    cursor.truncate().append_move("c5")

    assert game.mainline_move_count == 2


def test_cursor_append_game_returns_cursor_after_source_moves():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().to_game_end()
    source = _game_from_position(cursor.position, "Nf3")

    appended = cursor.append_game(source)

    assert appended.previous_move_san == "Nf3"


def test_cursor_append_game_requires_line_end():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()
    source = _game_from_position(cursor.position, "c5")

    with pytest.raises(ValueError, match="append_game requires cursor at line end"):
        cursor.append_game(source)


def test_cursor_add_variation_then_append_game_updates_variation_count():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()
    variation = cursor.add_variation()
    source = _game_from_position(variation.position, "c5")

    variation.append_game(source)

    assert cursor.variation_count == 1


def test_cursor_add_variation_then_append_game_stays_in_variation():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()
    variation = cursor.add_variation()
    source = _game_from_position(variation.position, "c5")

    appended = variation.append_game(source)

    assert appended.is_variation_line is True


def test_cursor_truncate_then_append_game_returns_cursor_after_source_moves():
    cursor = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *").create_cursor().next()
    source = _game_from_position(cursor.position, "c5")

    replaced = cursor.truncate().append_game(source)

    assert replaced.previous_move_san == "c5"


def test_cursor_truncate_then_append_game_replaces_suffix():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *")
    cursor = game.create_cursor().next()
    source = _game_from_position(cursor.position, "c5")

    cursor.truncate().append_game(source)

    assert game.mainline_move_count == 2


def test_cursor_truncate_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *").create_cursor().next()

    assert isinstance(cursor.truncate(), libscid.Cursor)


def test_cursor_truncate_removes_following_mainline_moves():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *")
    cursor = game.create_cursor().next()

    cursor.truncate()

    assert game.mainline_move_count == 1


def test_cursor_truncate_returns_line_end_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *").create_cursor().next()

    truncated = cursor.truncate()

    assert truncated.next_move_san is None


def test_cursor_truncate_is_visible_to_existing_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *").create_cursor().next()

    cursor.truncate()

    assert cursor.next_move_san is None


def test_cursor_truncate_removes_following_variation_moves():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5 2. c4) e5 *").create_cursor()
    variation = cursor.enter_variation(0).next()

    truncated = variation.truncate()

    assert truncated.next_move_san is None


def test_cursor_truncate_before_returns_new_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *").create_cursor().next()

    assert isinstance(cursor.truncate_before(), libscid.Cursor)


def test_cursor_truncate_before_removes_previous_mainline_moves():
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *")
    cursor = game.create_cursor().next()

    cursor.truncate_before()

    assert game.mainline_move_count == 2


def test_cursor_truncate_before_returns_line_start_cursor():
    cursor = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *").create_cursor().next()

    truncated = cursor.truncate_before()

    assert truncated.previous_move_san is None


def test_cursor_truncate_before_keeps_following_mainline_moves():
    cursor = libscid.Game.from_pgn("1. e4 e5 2. Nf3 *").create_cursor().next()

    truncated = cursor.truncate_before()

    assert truncated.next_move_uci == "e7e5"


def test_cursor_truncate_before_promotes_variation_suffix_to_mainline():
    cursor = libscid.Game.from_pgn("1. e4 (1. c4 c5) e5 *").create_cursor()
    variation = cursor.enter_variation(0).next()

    truncated = variation.truncate_before()

    assert truncated.next_move_uci == "c7c5"


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


def test_cursor_exposes_next_move_uci():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.next_move_uci == "e2e4"


def test_cursor_exposes_previous_move_uci():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert cursor.previous_move_uci == "e2e4"


def test_cursor_next_move_uci_returns_none_at_line_end():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()
    cursor = cursor.next()
    cursor = cursor.next()

    assert cursor.next_move_uci is None


def test_cursor_previous_move_uci_returns_none_at_line_start():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.previous_move_uci is None


def test_cursor_next_move_uci_reads_variation_line():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()

    assert cursor.enter_variation(0).next_move_uci == "d2d4"


def test_cursor_exposes_next_move_nags():
    cursor = libscid.Game.from_pgn("1. e4 $1 e5 *").create_cursor()

    assert cursor.next_move_nags == (libscid.Nag(1),)


def test_cursor_exposes_previous_move_nags():
    cursor = libscid.Game.from_pgn("1. e4 $1 e5 *").create_cursor().next()

    assert cursor.previous_move_nags == (libscid.Nag(1),)


def test_cursor_move_nags_preserve_order():
    cursor = libscid.Game.from_pgn("1. e4 $1 $146 e5 *").create_cursor()

    assert cursor.next_move_nags == (libscid.Nag(1), libscid.Nag(146))


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


def test_cursor_add_nag_returns_true_when_added():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert cursor.add_nag(libscid.Nag(1)) is True


def test_cursor_add_nag_updates_previous_move_nags():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    cursor.add_nag(libscid.Nag(1))

    assert cursor.previous_move_nags == (libscid.Nag(1),)


def test_cursor_add_nag_returns_false_at_line_start():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.add_nag(libscid.Nag(1)) is False


def test_cursor_add_nag_returns_false_for_zero_nag():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    assert cursor.add_nag(libscid.Nag(0)) is False


def test_cursor_add_nag_replaces_move_nag():
    cursor = libscid.Game.from_pgn("1. e4 $1 e5 *").create_cursor().next()

    cursor.add_nag(libscid.Nag(3))

    assert cursor.previous_move_nags == (libscid.Nag(3),)


def test_cursor_add_nag_replaces_position_nag():
    cursor = libscid.Game.from_pgn("1. e4 $10 e5 *").create_cursor().next()

    cursor.add_nag(libscid.Nag(14))

    assert cursor.previous_move_nags == (libscid.Nag(14),)


def test_cursor_remove_move_nag_removes_move_nag():
    cursor = libscid.Game.from_pgn("1. e4 $1 $10 e5 *").create_cursor().next()

    cursor.remove_move_nag()

    assert cursor.previous_move_nags == (libscid.Nag(10),)


def test_cursor_remove_move_nag_returns_false_without_move_nag():
    cursor = libscid.Game.from_pgn("1. e4 $10 e5 *").create_cursor().next()

    assert cursor.remove_move_nag() is False


def test_cursor_remove_position_nag_removes_position_nag():
    cursor = libscid.Game.from_pgn("1. e4 $1 $10 e5 *").create_cursor().next()

    cursor.remove_position_nag()

    assert cursor.previous_move_nags == (libscid.Nag(1),)


def test_cursor_remove_position_nag_returns_false_without_position_nag():
    cursor = libscid.Game.from_pgn("1. e4 $1 e5 *").create_cursor().next()

    assert cursor.remove_position_nag() is False


def test_cursor_remove_nags_removes_all_nags():
    cursor = libscid.Game.from_pgn("1. e4 $1 $10 e5 *").create_cursor().next()

    cursor.remove_nags()

    assert cursor.previous_move_nags == ()


def test_cursor_add_nag_is_visible_in_pgn_output():
    game = libscid.Game.from_pgn("1. e4 e5 *")
    cursor = game.create_cursor().next()

    cursor.add_nag(libscid.Nag(1))

    assert "$1" in game.to_pgn()


def test_cursor_preceding_comment_reads_initial_comment():
    cursor = libscid.Game.from_pgn("{Before game} 1. e4 e5 *").create_cursor()

    assert cursor.preceding_comment == "Before game"


def test_cursor_preceding_comment_reads_variation_initial_comment():
    cursor = libscid.Game.from_pgn("1. e4 ({Branch} 1. d4 d5) e5 *").create_cursor()

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


def test_cursor_set_comment_updates_initial_comment():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    cursor.set_comment("Before game")

    assert cursor.preceding_comment == "Before game"


def test_cursor_set_comment_updates_previous_move_comment():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor().next()

    cursor.set_comment("King pawn")

    assert cursor.comment == "King pawn"


def test_cursor_set_comment_updates_variation_initial_comment():
    cursor = libscid.Game.from_pgn("1. e4 (1. d4 d5) e5 *").create_cursor()
    variation = cursor.enter_variation(0)

    variation.set_comment("Queen pawn")

    assert variation.preceding_comment == "Queen pawn"


def test_cursor_remove_comment_removes_comment():
    cursor = libscid.Game.from_pgn("1. e4 {King pawn} e5 *").create_cursor().next()

    cursor.remove_comment()

    assert cursor.comment == ""


def test_cursor_set_comment_is_visible_in_pgn_output():
    game = libscid.Game.from_pgn("1. e4 e5 *")
    cursor = game.create_cursor().next()

    cursor.set_comment("King pawn")

    assert "{King pawn}" in game.to_pgn()


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
