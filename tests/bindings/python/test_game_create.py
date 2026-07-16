import libscid


def test_game_constructor_creates_blank_game():
    game = libscid.Game()

    assert game.mainline_move_count == 0


def test_blank_game_can_export_result_marker():
    game = libscid.Game()

    assert game.to_pgn().rstrip().endswith("*")


def test_game_constructor_can_create_blank_game_from_position():
    position = libscid.Game.from_pgn("1. e4 e5 *").end_position

    game = libscid.Game(position=position)

    assert game.start_position.fen == position.fen
