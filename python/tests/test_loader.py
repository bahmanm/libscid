from pathlib import Path

from libscid._native import _loader


def test_candidate_staging_directories_in_tree():
    staging_dirs = _loader._candidate_staging_directories()
    assert isinstance(staging_dirs, tuple)


def test_candidate_staging_directories_handles_shallow_paths(monkeypatch):
    shallow_file = Path("/app/libscid/_native/_loader.py")
    monkeypatch.setattr(_loader, "__file__", str(shallow_file))

    staging_dirs = _loader._candidate_staging_directories()
    assert staging_dirs == ()


def test_candidate_staging_directories_handles_root_path(monkeypatch):
    root_file = Path("/libscid/_native/_loader.py")
    monkeypatch.setattr(_loader, "__file__", str(root_file))

    staging_dirs = _loader._candidate_staging_directories()
    assert staging_dirs == ()


def test_candidate_staging_directories_handles_missing_staging_dir(
    tmp_path, monkeypatch
):
    deep_file = tmp_path / "a" / "b" / "c" / "libscid" / "_native" / "_loader.py"
    deep_file.parent.mkdir(parents=True)
    deep_file.touch()
    monkeypatch.setattr(_loader, "__file__", str(deep_file))

    staging_dirs = _loader._candidate_staging_directories()
    assert staging_dirs == ()
