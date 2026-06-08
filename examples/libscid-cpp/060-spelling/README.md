# 060 Spelling

This example loads a small spelling file, corrects a player-name alias, and
reads player metadata from the spelling database.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid-cpp/060-spelling -B _build/examples/libscid-cpp/060-spelling -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid-cpp/060-spelling
ctest --test-dir _build/examples/libscid-cpp/060-spelling --output-on-failure
```
