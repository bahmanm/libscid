# 80 Search Position

This example finds the Dr Lasker-Capablanca game in the Dr Lasker SCID5
database, takes a position from its main line, and searches the database for
games that reached that position.

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
cmake -S examples/80-search-position -B _build/examples/80-search-position -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/80-search-position
ctest --test-dir _build/examples/80-search-position --output-on-failure
```
