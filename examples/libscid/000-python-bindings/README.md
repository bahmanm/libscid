# Python ctypes Example

This directory contains a small Python wrapper over the `libscid` C ABI and a
PGN roundtrip script.

Build the shared library first:

```sh
cmake -S . -B _build-shared -DBUILD_SHARED_LIBS=ON
cmake --build _build-shared
```

Then run:

```sh
python3 examples/libscid/000-python-bindings/pgn_roundtrip.py
```

If the shared library is outside the default local build tree, pass it directly:

```sh
python3 examples/libscid/000-python-bindings/pgn_roundtrip.py --library /path/to/libscid.dylib
```

or set `LIBSCID_LIBRARY`.
