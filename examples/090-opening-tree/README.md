# 090 Opening Tree

This example finds the Dr Lasker-Capablanca game in the Dr Lasker SCID5
database, searches for a position from that game, and builds opening-tree
statistics for the matching games.

It reuses the SCID5 database from `examples/fixtures/dr-lasker/dr-lasker`.

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
cmake -S examples/090-opening-tree -B _build/examples/090-opening-tree -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/090-opening-tree
ctest --test-dir _build/examples/090-opening-tree --output-on-failure
```
