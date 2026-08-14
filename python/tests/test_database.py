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
