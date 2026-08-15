from __future__ import annotations

import pytest

import libscid


def write_pgn(path, game_count: int) -> None:
    games = []
    for index in range(game_count):
        number = index + 1
        games.append(
            f"""[Event "PGN {number}"]
[Site "Vancouver"]
[Date "2026.08.13"]
[Round "{number}"]
[White "White {number}"]
[Black "Black {number}"]
[Result "1-0"]

1. e4 e5 2. Nf3 1-0
"""
        )
    path.write_text("\n".join(games), encoding="utf-8")


def write_search_pgn(path) -> None:
    path.write_text(
        """[Event "World Championship"]
[Site "Havana"]
[Date "1921.04.01"]
[Round "1"]
[White "Dr. Lasker"]
[Black "Capa"]
[Result "0-1"]

1. e4 e5 2. Nf3 Nc6 0-1

[Event "World Championship"]
[Site "Havana"]
[Date "1921.04.02"]
[Round "2"]
[White "Capa"]
[Black "Dr. Lasker"]
[Result "1/2-1/2"]

1. d4 d5 2. c4 e6 1/2-1/2

[Event "Other"]
[Site "Vancouver"]
[Date "1922.01.01"]
[Round "1"]
[White "Alekhine"]
[Black "Bogoljubow"]
[Result "1-0"]

1. e4 e5 2. Nf3 Nc6 1-0
""",
        encoding="utf-8",
    )


def write_board_search_pgn(path) -> None:
    path.write_text(
        """[Event "E Pawn"]
[Site "Toronto"]
[Date "2024.06.14"]
[White "Alpha"]
[Black "Beta"]
[Result "1-0"]

1. e4 e5 2. Nf3 1-0

[Event "D Pawn"]
[Site "Vancouver"]
[Date "2025.01.02"]
[White "Gamma"]
[Black "Delta"]
[Result "0-1"]

1. d4 d5 0-1

[Event "English"]
[Site "Montreal"]
[Date "2026.02.03"]
[White "Epsilon"]
[Black "Zeta"]
[Result "1/2-1/2"]

1. c4 c5 1/2-1/2

[Event "Indian"]
[Site "Calgary"]
[Date "2026.02.04"]
[White "Eta"]
[Black "Theta"]
[Result "0-1"]

1. d4 Nf6 0-1

[Event "Variation"]
[Site "Victoria"]
[Date "2027.01.01"]
[White "Iota"]
[Black "Kappa"]
[Result "*"]

1. a3 (1. h4 h5) a6 *
""",
        encoding="utf-8",
    )


def test_database_open_pgn_read_only_loads_games(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 2)

    database = libscid.Database.open_pgn_read_only(path)

    assert database.type == "PGN"
    assert database.read_only is True
    assert database.game_count == 2
    assert database.get_tag(1, "Event") == "PGN 2"


def test_database_get_game_materialises_game(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 1)
    database = libscid.Database.open_pgn_read_only(path)

    game = database.get_game(0)

    assert game.get_tag("Event") == "PGN 1"
    assert game.mainline_move_count == 3


def test_database_exposes_all_games_filter(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 3)
    database = libscid.Database.open_pgn_read_only(path)

    assert isinstance(database.filters, libscid.DatabaseFilters)
    assert database.filters is database.filters

    filter_ = database.filters.all_games

    assert isinstance(filter_, libscid.Filter)
    assert filter_.game_count == 3
    assert filter_.get_game_indices("N+", 0, 3) == (0, 1, 2)
    assert filter_.get_game_indices("N+", 1, 2) == (1, 2)
    assert filter_.get_game_indices() == (0, 1, 2)
    assert filter_.get_game_index_at_row(2, "N+") == 2
    assert filter_.get_game_row_for_index(2, "N+") == 2


def test_database_exposes_primary_filter(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 2)
    database = libscid.Database.open_pgn_read_only(path)

    filter_ = database.filters.primary

    assert isinstance(filter_, libscid.Filter)
    assert filter_.game_count == 2
    assert filter_.get_game_indices("N+", 0, 2) == (0, 1)


def test_database_can_create_filter(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 2)
    database = libscid.Database.open_pgn_read_only(path)

    filter_ = database.filters.create()

    assert isinstance(filter_, libscid.Filter)
    assert filter_.game_count == 2
    assert filter_.get_game_indices("N+", 0, 2) == (0, 1)


def test_created_filter_can_be_deleted(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 1)
    database = libscid.Database.open_pgn_read_only(path)
    filter_ = database.filters.create()

    filter_.delete()

    with pytest.raises(ValueError, match="filter has been deleted"):
        _ = filter_.game_count


def test_builtin_filters_cannot_be_deleted(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 1)
    database = libscid.Database.open_pgn_read_only(path)

    with pytest.raises(ValueError, match="built-in filters cannot be deleted"):
        database.filters.all_games.delete()


def test_filter_rejects_negative_rows(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 1)
    database = libscid.Database.open_pgn_read_only(path)
    filter_ = database.filters.all_games

    with pytest.raises(ValueError, match="start_row must be non-negative"):
        filter_.get_game_indices(start_row=-1)
    with pytest.raises(ValueError, match="row_count must be non-negative"):
        filter_.get_game_indices(row_count=-1)
    with pytest.raises(ValueError, match="row must be non-negative"):
        filter_.get_game_index_at_row(-1)
    with pytest.raises(ValueError, match="game_index must be non-negative"):
        filter_.get_game_row_for_index(-1)


def test_database_search_exposes_header_criteria_from_search_namespace(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)

    criteria = database.search.HeaderCriteria(white="Dr. Lasker", black="Capa")
    criteria.event = "World Championship"

    matches = database.search.headers(criteria)

    assert isinstance(database.search, libscid.DatabaseSearch)
    assert database.search is database.search
    assert isinstance(matches, libscid.Filter)
    assert matches.get_game_indices("N+") == (0,)


def test_database_search_can_search_within_source_filter(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)

    decisive = database.search.headers(
        database.search.HeaderCriteria(result=["1-0", "0-1"])
    )
    world_championship_decisive = database.search.headers(
        database.search.HeaderCriteria(event="World Championship"),
        source=decisive,
    )

    assert decisive.get_game_indices("N+") == (0, 2)
    assert world_championship_decisive.get_game_indices("N+") == (0,)


def test_database_search_can_reuse_destination_filter(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    destination = database.filters.create()

    matches = database.search.headers(
        database.search.HeaderCriteria(result="1/2-1/2"),
        destination=destination,
    )

    assert matches is destination
    assert destination.get_game_indices("N+") == (1,)


def test_database_search_can_search_position(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"
    )

    matches = database.search.position(position)

    assert isinstance(matches, libscid.Filter)
    assert matches.get_game_indices("N+") == (0, 2)


def test_database_search_position_can_search_within_source_filter(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"
    )
    white_wins = database.search.headers(database.search.HeaderCriteria(result="1-0"))
    destination = database.filters.create()

    matches = database.search.position(
        position,
        source=white_wins,
        destination=destination,
    )

    assert matches is destination
    assert destination.get_game_indices("N+") == (2,)


def test_database_search_board_exposes_match_constants(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2"
    )

    matches = database.search.board(
        position,
        match=database.search.BOARD_MATCH_EXACT,
    )

    assert database.search.BOARD_MATCH_EXACT == "exact"
    assert database.search.BOARD_MATCH_PAWNS == "pawns"
    assert database.search.BOARD_MATCH_FILES == "files"
    assert matches.get_game_indices("N+") == (1,)


def test_database_search_board_can_search_within_source_filter(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
    )
    black_wins = database.search.headers(database.search.HeaderCriteria(result="0-1"))
    destination = database.filters.create()

    matches = database.search.board(
        position,
        match=database.search.BOARD_MATCH_FILES,
        source=black_wins,
        destination=destination,
    )

    assert matches is destination
    assert destination.get_game_indices("N+") == (1, 3)


def test_database_search_board_can_use_pawn_and_file_matches(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    d4_d5 = libscid.Position.from_fen(
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2"
    )
    e4_files = libscid.Position.from_fen(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"
    )

    pawn_matches = database.search.board(d4_d5, match="pawns")
    file_matches = database.search.board(
        e4_files,
        match=database.search.BOARD_MATCH_FILES,
    )

    assert pawn_matches.get_game_indices("N+") == (1,)
    assert file_matches.get_game_indices("N+") == (0, 1, 2, 3, 4)


def test_database_search_board_can_include_flipped_position(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    flipped = libscid.Position.from_fen(
        "rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    )

    without_flipped = database.search.board(flipped)
    with_flipped = database.search.board(flipped, include_flipped=True)

    assert without_flipped.get_game_indices("N+") == ()
    assert with_flipped.get_game_indices("N+") == (0,)


def test_database_search_board_can_include_variations(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    variation_position = libscid.Position.from_fen(
        "rnbqkbnr/ppppppp1/8/7p/7P/8/PPPPPPP1/RNBQKBNR w KQkq - 0 2"
    )

    without_variations = database.search.board(variation_position)
    with_variations = database.search.board(variation_position, include_variations=True)

    assert without_variations.get_game_indices("N+") == ()
    assert with_variations.get_game_indices("N+") == (4,)


def test_database_search_reports_progress(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    reports = []

    def report(done: int, total: int, message: str | None) -> None:
        reports.append((done, total, message))

    database.search.headers(
        database.search.HeaderCriteria(result="1-0"),
        progress_report_callback=report,
    )

    assert reports


def test_database_search_position_reports_progress(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"
    )
    reports = []

    def report(done: int, total: int, message: str | None) -> None:
        reports.append((done, total, message))

    database.search.position(position, progress_report_callback=report)

    assert reports


def test_database_search_board_reports_progress(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2"
    )
    reports = []

    def report(done: int, total: int, message: str | None) -> None:
        reports.append((done, total, message))

    database.search.board(position, progress_report_callback=report)

    assert reports


def test_database_search_can_be_cancelled(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    calls = 0

    def should_cancel() -> bool:
        nonlocal calls
        calls += 1
        return True

    with pytest.raises(libscid.LibScidError) as raised:
        database.search.headers(
            database.search.HeaderCriteria(result="1-0"),
            should_cancel_fn=should_cancel,
        )

    assert raised.value.code == 2
    assert calls > 0


def test_database_search_position_can_be_cancelled(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"
    )
    calls = 0

    def should_cancel() -> bool:
        nonlocal calls
        calls += 1
        return True

    with pytest.raises(libscid.LibScidError) as raised:
        database.search.position(position, should_cancel_fn=should_cancel)

    assert raised.value.code == 2
    assert calls > 0


def test_database_search_board_can_be_cancelled(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2"
    )
    calls = 0

    def should_cancel() -> bool:
        nonlocal calls
        calls += 1
        return True

    with pytest.raises(libscid.LibScidError) as raised:
        database.search.board(position, should_cancel_fn=should_cancel)

    assert raised.value.code == 2
    assert calls > 0


def test_database_search_reraises_progress_callback_exception(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)

    def report(_done: int, _total: int, _message: str | None) -> None:
        raise RuntimeError("stop from Python")

    with pytest.raises(RuntimeError, match="stop from Python"):
        database.search.headers(
            database.search.HeaderCriteria(result="1-0"),
            progress_report_callback=report,
        )


def test_database_search_position_reraises_progress_callback_exception(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"
    )

    def report(_done: int, _total: int, _message: str | None) -> None:
        raise RuntimeError("stop from Python")

    with pytest.raises(RuntimeError, match="stop from Python"):
        database.search.position(position, progress_report_callback=report)


def test_database_search_board_reraises_progress_callback_exception(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2"
    )

    def report(_done: int, _total: int, _message: str | None) -> None:
        raise RuntimeError("stop from Python")

    with pytest.raises(RuntimeError, match="stop from Python"):
        database.search.board(position, progress_report_callback=report)


def test_database_search_validates_criteria(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)

    with pytest.raises(ValueError, match="game_number_min must be non-negative"):
        database.search.headers(
            database.search.HeaderCriteria(game_number_min=-1),
        )
    with pytest.raises(TypeError, match="has_comments must be bool"):
        database.search.headers(
            database.search.HeaderCriteria(has_comments="yes"),
        )
    with pytest.raises(TypeError, match="result items must be str"):
        database.search.headers(
            database.search.HeaderCriteria(result=["1-0", 1]),
        )
    with pytest.raises(ValueError, match="destination cannot be the all_games filter"):
        database.search.headers(
            database.search.HeaderCriteria(result="1-0"),
            destination=database.filters.all_games,
        )


def test_database_search_position_validates_arguments(tmp_path):
    path = tmp_path / "games.pgn"
    write_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"
    )

    with pytest.raises(TypeError, match="position must be a Position"):
        database.search.position("not a position")
    with pytest.raises(ValueError, match="destination cannot be the all_games filter"):
        database.search.position(position, destination=database.filters.all_games)


def test_database_search_board_validates_arguments(tmp_path):
    path = tmp_path / "games.pgn"
    write_board_search_pgn(path)
    database = libscid.Database.open_pgn_read_only(path)
    position = libscid.Position.from_fen(
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2"
    )

    with pytest.raises(TypeError, match="position must be a Position"):
        database.search.board("not a position")
    with pytest.raises(TypeError, match="match must be str"):
        database.search.board(position, match=1)
    with pytest.raises(ValueError, match="match must be 'exact', 'pawns', or 'files'"):
        database.search.board(position, match="material")
    with pytest.raises(TypeError, match="include_variations must be bool"):
        database.search.board(position, include_variations="yes")
    with pytest.raises(TypeError, match="include_flipped must be bool"):
        database.search.board(position, include_flipped="yes")
    with pytest.raises(ValueError, match="destination cannot be the all_games filter"):
        database.search.board(position, destination=database.filters.all_games)


def test_database_open_pgn_read_only_reports_progress(tmp_path):
    path = tmp_path / "games.pgn"
    write_pgn(path, 2)
    reports = []

    def report(done: int, total: int, message: str | None) -> None:
        reports.append((done, total, message))

    libscid.Database.open_pgn_read_only(path, progress_report_callback=report)

    assert reports
    assert reports[-1][0:2] == (1, 1)
    assert reports[-1][2] is not None


def test_database_open_pgn_read_only_can_be_cancelled(tmp_path):
    path = tmp_path / "large.pgn"
    write_pgn(path, 1030)
    calls = 0

    def should_cancel() -> bool:
        nonlocal calls
        calls += 1
        return True

    with pytest.raises(libscid.LibScidError) as raised:
        libscid.Database.open_pgn_read_only(path, should_cancel=should_cancel)

    assert raised.value.code == 2
    assert calls > 0


def test_database_open_pgn_read_only_reraises_progress_callback_exception(tmp_path):
    path = tmp_path / "large.pgn"
    write_pgn(path, 1030)

    def report(_done: int, _total: int, _message: str | None) -> None:
        raise RuntimeError("stop from Python")

    with pytest.raises(RuntimeError, match="stop from Python"):
        libscid.Database.open_pgn_read_only(path, progress_report_callback=report)
