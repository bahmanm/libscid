from dataclasses import FrozenInstanceError

import pytest

import libscid


def test_nag_constructor_reads_symbolic_nag():
    assert libscid.Nag("!") == libscid.Nag(1)


def test_nag_constructor_reads_numeric_nag():
    assert libscid.Nag("$146") == libscid.Nag(146)


def test_nag_constructor_reads_bytes_nag():
    assert libscid.Nag(b"!") == libscid.Nag(1)


def test_nag_constructor_reads_unknown_nag_as_zero():
    assert libscid.Nag("not-a-nag") == libscid.Nag(0)


def test_nag_exposes_code():
    assert libscid.Nag(1).code == 1


def test_nag_exposes_text():
    assert libscid.Nag(1).text == "$1"


def test_nag_exposes_symbol():
    assert libscid.Nag(1).symbol == "!"


def test_nag_exposes_non_move_annotation_symbol():
    assert libscid.Nag(146).symbol == "N"


def test_nag_formats_as_text():
    assert str(libscid.Nag(1)) == "$1"


def test_nag_zero_text_is_empty_string():
    assert libscid.Nag(0).text == ""


def test_nag_rejects_negative_code():
    with pytest.raises(ValueError, match="Nag code must be between 0 and 255"):
        libscid.Nag(-1)


def test_nag_rejects_large_code():
    with pytest.raises(ValueError, match="Nag code must be between 0 and 255"):
        libscid.Nag(256)


def test_nag_is_immutable():
    nag = libscid.Nag(1)

    with pytest.raises(FrozenInstanceError):
        nag.code = 2
