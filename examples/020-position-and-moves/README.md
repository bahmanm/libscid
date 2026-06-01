# 020 Position and Moves

This example starts from the standard chess position, applies coordinate moves,
prints the resulting FEN, and generates legal replies from that position.

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
cmake -S examples/020-position-and-moves -B _build/examples/020-position-and-moves -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/020-position-and-moves
ctest --test-dir _build/examples/020-position-and-moves --output-on-failure
```
