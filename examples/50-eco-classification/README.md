# 50 ECO Classification

This example loads a small ECO book, plays a few moves on a
`scid::core::Position`, and classifies the resulting opening position.

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
cmake -S examples/50-eco-classification -B _build/examples/50-eco-classification -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/50-eco-classification
ctest --test-dir _build/examples/50-eco-classification --output-on-failure
```
