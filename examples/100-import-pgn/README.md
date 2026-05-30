# 100 Import PGN

This example opens an in-memory database, imports games from a PGN file, and
loads one imported game back through the database API.

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
cmake -S examples/100-import-pgn -B _build/examples/100-import-pgn -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/100-import-pgn
ctest --test-dir _build/examples/100-import-pgn --output-on-failure
```
