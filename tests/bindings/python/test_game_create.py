import libscid


def test_game_constructor_creates_blank_game():
    game = libscid.Game()

    assert game.mainline_halfmove_count == 0


def test_blank_game_can_export_result_marker():
    game = libscid.Game()

    assert game.to_pgn().rstrip().endswith("*")
