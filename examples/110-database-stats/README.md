# 110 Database Stats

This example opens the Dr Lasker SCID5 database and reads aggregate database
statistics: date range, result buckets, rating coverage, and ECO coverage.

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
cmake -S examples/110-database-stats -B _build/examples/110-database-stats -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/110-database-stats
ctest --test-dir _build/examples/110-database-stats --output-on-failure
```
