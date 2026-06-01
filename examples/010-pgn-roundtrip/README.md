# 010 PGN Roundtrip

This example parses the famous Lasker-Capablanca, St Petersburg 1914 game into
a `scid::core::Game`, reads a few fields, and encodes it back to PGN.

The only prerequisite is to have a libscid release already installed. For instance:

```sh
platform=linux ## linux, macos-arm, or windows
version=v0.1.0
asset_name="libscid__${version}__${platform}.tar.gz"

mkdir -p install/libscid downloads/
curl -L \
    https://github.com/bahmanm/libscid/releases/latest/download/${asset_name} \
    -o downloads/$asset_name

tar -xzf \
    downloads/$asset_name \
    -C install/libscid/ \
    --strip-components=1
```

```sh
cmake -S examples/010-pgn-roundtrip -B _build/examples/010-pgn-roundtrip -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/010-pgn-roundtrip
ctest --test-dir _build/examples/010-pgn-roundtrip --output-on-failure
```
