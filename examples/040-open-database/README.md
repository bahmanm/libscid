# 040 Open Database

This example opens a SCID5 database in read-only mode and prints a small summary
of its first game.

It expects the shared SCID5 fixture at `examples/fixtures/dr-lasker/dr-lasker`,
made from the three files `dr-lasker.si5`, `dr-lasker.sg5`, and `dr-lasker.sn5`.

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
cmake -S examples/040-open-database -B _build/examples/040-open-database -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/040-open-database
ctest --test-dir _build/examples/040-open-database --output-on-failure
```
