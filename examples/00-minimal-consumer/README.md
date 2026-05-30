# 00 Minimal Consumer

This is the 'hello, world' of libscid.

The idea is to demonstrate how a C++ application can discover and link the installed
libscid CMake package; it intentionally does little work with the API. Later examples cover PGN,
positions, databases, ECO, and spelling in more depth.

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
cmake -S examples/00-minimal-consumer -B _build/examples/00-minimal-consumer -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/00-minimal-consumer
ctest --test-dir _build/examples/00-minimal-consumer --output-on-failure
```
