import libscid

REPETITION_PGN = "1. Nf3 Nf6 2. Ng1 Ng8 3. Nf3 Nf6 4. Ng1 Ng8 *"
REPETITION_VARIATION_PGN = (
    "1. e4 e5 2. Nc3 (2. Nf3 Nc6 3. Ng1 Nb8 4. Nf3 Nc6 5. Ng1 Nb8) Nc6 *"
)


def test_cursor_exposes_arbiter():
    cursor = libscid.Game.from_pgn("1. e4 e5 *").create_cursor()

    assert cursor.arbiter.can_claim_fifty_move_rule is False


def test_arbiter_can_claim_fifty_move_rule_at_halfmove_threshold():
    position = libscid.Position.from_fen("8/K7/8/8/7k/8/8/8 w - - 100 51")
    cursor = libscid.Game(position).create_cursor()

    assert cursor.arbiter.can_claim_fifty_move_rule is True


def test_arbiter_cannot_claim_fifty_move_rule_before_halfmove_threshold():
    position = libscid.Position.from_fen("8/K7/8/8/7k/8/8/8 w - - 99 50")
    cursor = libscid.Game(position).create_cursor()

    assert cursor.arbiter.can_claim_fifty_move_rule is False


def test_arbiter_can_claim_threefold_repetition_on_main_line():
    cursor = libscid.Game.from_pgn(REPETITION_PGN).create_cursor().to_game_end()

    assert cursor.arbiter.can_claim_threefold_repetition is True


def test_arbiter_cannot_claim_threefold_repetition_before_third_occurrence():
    cursor = libscid.Game.from_pgn(REPETITION_PGN).create_cursor()
    for _ in range(4):
        cursor = cursor.next()

    assert cursor.arbiter.can_claim_threefold_repetition is False


def test_arbiter_can_claim_threefold_repetition_in_variation():
    cursor = libscid.Game.from_pgn(REPETITION_VARIATION_PGN).create_cursor()
    cursor = cursor.next().next()
    variation = cursor.enter_variation(0)
    while not variation.is_line_end:
        variation = variation.next()

    assert variation.arbiter.can_claim_threefold_repetition is True
