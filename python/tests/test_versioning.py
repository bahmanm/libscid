from __future__ import annotations

import sys
from pathlib import Path

import pytest

# Ensure hatch_build is importable from python/ directory
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from hatch_build import to_pep440


@pytest.mark.parametrize(
    ("input_tag", "expected_pep440"),
    [
        ("v1.0.0", "1.0.0"),
        ("1.0.0", "1.0.0"),
        ("v0.10.0-testing.5", "0.10.0.dev5"),
        ("0.10.0-testing.1", "0.10.0.dev1"),
        ("v1.0.0-test.3", "1.0.0.dev3"),
        ("v1.0.0-dev.2", "1.0.0.dev2"),
        ("v1.0.0-dev2", "1.0.0.dev2"),
        ("v1.0.0-alpha.1", "1.0.0a1"),
        ("v1.0.0-a.2", "1.0.0a2"),
        ("v1.0.0-a1", "1.0.0a1"),
        ("v1.0.0-beta.1", "1.0.0b1"),
        ("1.0.0-beta.1", "1.0.0b1"),
        ("v1.0.0-b.1", "1.0.0b1"),
        ("v1.0.0-b3", "1.0.0b3"),
        ("v1.0.0-rc.1", "1.0.0rc1"),
        ("v1.0.0-rc2", "1.0.0rc2"),
        ("v1.0.0-post.1", "1.0.0.post1"),
        ("0.10.0b1", "0.10.0b1"),
        ("0.10.0.dev5", "0.10.0.dev5"),
    ],
)
def test_to_pep440_normalization(input_tag: str, expected_pep440: str) -> None:
    assert to_pep440(input_tag) == expected_pep440
