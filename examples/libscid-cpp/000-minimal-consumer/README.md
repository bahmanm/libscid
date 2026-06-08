# 000 Minimal Consumer

This is the 'hello, world' of libscid.

The idea is to demonstrate how a C++ application can discover and link the installed
libscid CMake package; it intentionally does little work with the API. Later examples cover PGN,
positions, databases, ECO, and spelling in more depth.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid-cpp/000-minimal-consumer -B _build/examples/libscid-cpp/000-minimal-consumer -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid-cpp/000-minimal-consumer
ctest --test-dir _build/examples/libscid-cpp/000-minimal-consumer --output-on-failure
```
