# 060 Mutate PGN

This example parses an existing PGN game through the C ABI, navigates to a
move with a movetext cursor, edits its comment and NAGs, adds a new variation,
promotes that variation, and writes the changed PGN.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/libscid/060-mutate-pgn -B _build/examples/libscid/060-mutate-pgn -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/libscid/060-mutate-pgn
ctest --test-dir _build/examples/libscid/060-mutate-pgn --output-on-failure
```
