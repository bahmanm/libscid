import libscid
import pytest


TAGGED_PGN = """[Event "Friendly"]
[Site "Toronto"]
[Date "2024.05.01"]
[Round "1"]
[White "Player A"]
[Black "Player B"]
[Result "*"]
[Annotator "Example"]

1. e4 e5 *
"""


def test_game_can_read_roster_tag():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    assert game.get_tag("Event") == "Friendly"


def test_game_returns_empty_string_for_missing_tag():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    assert game.get_tag("Missing") == ""


def test_game_can_set_roster_tag():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    game.set_tag("Event", "Edited")

    assert game.get_tag("Event") == "Edited"


def test_game_can_set_supplemental_tag():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    game.set_tag("ECO", "C20")

    assert game.get_tag("ECO") == "C20"


def test_game_rejects_invalid_result_tag():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    with pytest.raises(libscid.LibScidError):
        game.set_tag("Result", "bad-result")


def test_game_can_remove_supplemental_tag():
    game = libscid.Game.from_pgn(TAGGED_PGN)
    game.set_tag("ECO", "C20")

    game.remove_tag("ECO")

    assert game.get_tag("ECO") == ""


def test_game_remove_tag_reports_removed_tag():
    game = libscid.Game.from_pgn(TAGGED_PGN)
    game.set_tag("ECO", "C20")

    assert game.remove_tag("ECO") is True


def test_game_remove_tag_reports_missing_tag():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    assert game.remove_tag("ECO") is False


def test_game_remove_tag_reports_roster_tag_not_removed():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    assert game.remove_tag("Event") is False


def test_game_get_tags_returns_ordered_snapshot():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    assert game.get_tags() == (
        ("Event", "Friendly"),
        ("Site", "Toronto"),
        ("Date", "2024.05.01"),
        ("Round", "1"),
        ("White", "Player A"),
        ("Black", "Player B"),
        ("Result", "*"),
        ("Annotator", "Example"),
    )


def test_game_get_tags_supports_list_comprehensions():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    assert [name for name, _ in game.get_tags()] == [
        "Event",
        "Site",
        "Date",
        "Round",
        "White",
        "Black",
        "Result",
        "Annotator",
    ]


def test_game_get_tags_returns_read_only_snapshot():
    game = libscid.Game.from_pgn(TAGGED_PGN)

    with pytest.raises(AttributeError):
        game.get_tags().append(("ECO", "C20"))
