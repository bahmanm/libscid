# 060 Spelling

This example loads a small spelling file, corrects a player-name alias, and
reads player metadata from the spelling database.

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
cmake -S examples/060-spelling -B _build/examples/060-spelling -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/060-spelling
ctest --test-dir _build/examples/060-spelling --output-on-failure
```
