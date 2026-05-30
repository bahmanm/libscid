# libscid Examples

Small standalone projects as examples to demonstrate how to use libscid.
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

Then you would configure the examples with `CMAKE_PREFIX_PATH` pointing at the installation path.

```sh
cmake -S examples -B _build/examples -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples
ctest --test-dir _build/examples --output-on-failure
```
