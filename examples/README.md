# libscid Examples

These are small standalone projects showing how to consume the installed libscid
packages.

The examples are organised by public API surface:

- `libscid/`: examples for the C ABI.
- `libscid-cpp/`: examples for the native C++ API.

The canonical examples and recipes index lives in the generated documentation:

- https://libscid.bahmanm.com/examples_recipes.html

After installing libscid, configure and run the whole example suite with:

```sh
cmake -S examples -B _build/examples -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples
ctest --test-dir _build/examples --output-on-failure
```

See the installation guide for release archive and source-install workflows:

- https://libscid.bahmanm.com/installation.html
