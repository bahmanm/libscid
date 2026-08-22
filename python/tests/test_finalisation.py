"""Tests for resource finalisation, disposal idempotency, and interpreter shutdown."""

from __future__ import annotations

import gc
import os
import subprocess
import sys
import tempfile
from pathlib import Path

import libscid


def test_game_finaliser_and_disposal() -> None:
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")
    assert game.mainline_move_count == 4
    assert hasattr(game, "_finalizer")
    assert game._finalizer.alive

    # Explicit disposal is idempotent
    game._dispose()
    assert not game._finalizer.alive
    game._dispose()


def test_cursor_finaliser_and_disposal() -> None:
    game = libscid.Game.from_pgn("1. e4 e5 2. Nf3 Nc6 *")
    cursor = game.create_cursor()
    assert hasattr(cursor, "_finalizer")
    assert cursor._finalizer.alive

    # Explicit disposal is idempotent
    cursor._dispose()
    assert not cursor._finalizer.alive
    cursor._dispose()


def test_position_finaliser_and_disposal() -> None:
    position = libscid.Position.from_fen(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    )
    assert hasattr(position, "_finalizer")
    assert position._finalizer.alive

    # Explicit disposal is idempotent
    position._dispose()
    assert not position._finalizer.alive
    position._dispose()


def test_database_finaliser_and_disposal() -> None:
    pgn_data = '[Event "Test"]\n[White "W"]\n[Black "B"]\n[Result "1-0"]\n\n1. e4 1-0\n'
    with tempfile.NamedTemporaryFile("w+", suffix=".pgn", delete=False) as f:
        f.write(pgn_data)
        f.flush()
        path = f.name

    try:
        db = libscid.Database.open_pgn_read_only(path)
        assert hasattr(db, "_finalizer")
        assert db._finalizer.alive
        db.close()
        # Explicit disposal after close is idempotent
        db._dispose()
        assert not db._finalizer.alive
    finally:
        Path(path).unlink(missing_ok=True)


def test_garbage_collection_finalisation() -> None:
    def create_and_orphan() -> None:
        game = libscid.Game.from_pgn("1. d4 d5 2. c4 e6 *")
        cursor = game.create_cursor()
        _ = cursor.next()
        _ = libscid.Position.from_fen("8/8/8/8/8/8/8/4K2k w - - 0 1")

    create_and_orphan()
    gc.collect()


def test_interpreter_shutdown_subprocesses() -> None:
    """Executes a subprocess verifying clean interpreter shutdown with live objects."""
    script = """
import libscid

pgn_data = '''[Event "World Championship"]
[Site "Reykjavik ISL"]
[Date "1972.07.23"]
[Round "6"]
[White "Fischer, Robert J."]
[Black "Spassky, Boris V."]
[Result "1-0"]

1. c4 e6 2. Nf3 d5 3. d4 Nf6 4. Nc3 Be7 5. Bg5 O-O 6. e3 h6 7. Bh4 b6 1-0
'''

game = libscid.Game.from_pgn(pgn_data)
cursor = game.create_cursor()
while not cursor.is_line_end:
    cursor = cursor.next()
"""
    env = os.environ.copy()
    result = subprocess.run(
        [sys.executable, "-c", script],
        capture_output=True,
        text=True,
        env=env,
    )
    assert result.returncode == 0, (
        f"Script failed with code {result.returncode}:\n{result.stderr}"
    )
    assert "Exception ignored" not in result.stderr
    assert "TypeError" not in result.stderr
    assert result.stderr.strip() == ""
