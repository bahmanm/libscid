# 020 Author PGN

This example authors a PGN game through the C ABI, using a game cursor to
add moves, comments, NAGs, and a variation before writing the result as PGN.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid/020-author-pgn -B _build/examples/libscid/020-author-pgn -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid/020-author-pgn
ctest --test-dir _build/examples/libscid/020-author-pgn --output-on-failure
```
