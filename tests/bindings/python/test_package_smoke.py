import libscid


def test_libscid_package_exposes_public_api():
    assert libscid.__version__ == "0.0.0"
    assert libscid.Game is not None
    assert libscid.PgnOptions is not None
