# 70 List Database

This example opens the Dr Lasker SCID5 database, creates a sort cache, and lists
the first few games in date order.

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
cmake -S examples/70-list-database -B _build/examples/70-list-database -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/70-list-database
ctest --test-dir _build/examples/70-list-database --output-on-failure
```
